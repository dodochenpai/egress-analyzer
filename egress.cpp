#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <future>

using namespace std;

WSADATA wsaData;
SOCKET wSock;
struct sockaddr_in sockaddr;

int test_port(int port);
int thread_handler(vector<int> ports);
void print_usage();

int main(int argc, char *argv[]){
    // Create default variable values
    vector<future<int>> futures;
    vector<int> ports;
    int numThreads = 10; 
    int portStart;
    int portEnd;

    // Convert command line arguments to strings
    vector<string> argList(argv, argv + argc);

    // Parse Inputs
    for (int i=0;i<argList.size()-1;i++){
        if (argList[i] == "-p"){
            string portArg = argList[i+1];
            // Split argument by commas
            vector<string> substrings;
            int split = 0;
            for (int j=0;j<portArg.length();j++){
                if (portArg[j] == ','){
                    substrings.push_back(portArg.substr(split, j));
                    split = j+1;
                }
            }
            // Get last argument
            substrings.push_back(portArg.substr(split, portArg.size()));

            // Parse dashes and generate a list of ports to scan
            for (int j=0;j<substrings.size();j++){
                // If dash in comma separated argument, generate range
                size_t dashPos = substrings[j].find('-');
                if (dashPos != string::npos){
                    portStart = stoi(substrings[j].substr(0,dashPos));
                    portEnd = stoi(substrings[j].substr(dashPos+1, substrings[j].length()-dashPos));
                    for (int k=portStart;k<portEnd+1;k++){
                        ports.push_back(k);
                    }
                // Otherwise, add single port 
                } else {
                    portStart = stoi(substrings[j]);
                    portEnd = stoi(substrings[j]);
                    for (int k=portStart;k<portEnd+1;k++){
                        ports.push_back(k);
                    }
                }
            }
        } else if (argList[i] == "-t"){
            string threadArg = argList[i+1];
            for (int j=0;j<threadArg.length();j++){
                numThreads = stoi(threadArg);
            }
        } else if (argList[i] == "-h"){
            print_usage();
            return 0;
        }
    }

    // Initialize WinSock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup Failed" << endl;
        return 1;
    }

    // Default scan ports 1-1024
    if (ports.size() == 0){
        for (int i=1;i<1025;i++){
            ports.push_back(i);
        }
    }

    // Divide ports between threads
    vector<vector<int>> threadTasks(numThreads, vector<int>(0,0));
    for (int i=0; i<ports.size();i++){
        threadTasks[i%numThreads].push_back(ports[i]);
    }

    //cout << "Scanning ports " << portStart << '-' << portEnd << endl;
    // Create threads
    cout << "Starting " << numThreads << " thread(s)\n";
    for(int i=0;i<numThreads;i++){
        // threads.push_back(thread(thread_handler, i+portStart, numThreads, portEnd));
        futures.push_back(async(thread_handler, threadTasks[i]));
    }

    // Wait for threads to finish before exiting
    for (int i=0;i<numThreads;i++){
        //threads[i].join();
        futures[i].wait();
    }
    int openCount = 0;
    for (int i=0; i<futures.size();i++){
        openCount += futures[i].get();
    }
    // Print out some statistics
    cout << "Open ports: " << openCount << endl;
    // cout << "See outfile for more output" << endl;

    WSACleanup();
    return 0;

}

// Calculates the ports each thread will scan
// Returns total number of open ports
int thread_handler(vector<int> ports){
    // Set variable to initial value
    int openCount = 0;
    int closedCount = 0;
    int retvalue;
    for (int i=0;i<ports.size();i++){
        // Scan the port
        retvalue = test_port(ports[i]);
        // Retry if port is closed
        
        for (int j=0;j<2;j++){
            if (retvalue == 0){
                openCount++;
                break;
            } else {
                cout << "Connection failed on port " << ports[i] << endl;
                retvalue = test_port(ports[i]);
            }
            closedCount++;
        }
        // Set next port to scan
    }
    return openCount;
}

int test_port(int port){
    // Initialize Variables
    SOCKET wSock;
    struct sockaddr_in sockaddr;
    struct timeval tv;
    int result;
    int retvalue = 1;

    // Set Options
    // Google Test
    // string target = "8.8.8.8";
    // allports.exposed
    string target = "45.79.204.144";
    // Localhost test
    // string target = "127.0.0.1";
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(target.c_str());
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    // Create Socket
    wSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (wSock == INVALID_SOCKET) {
        cout << "Socket Error" << endl;
        closesocket(wSock);
        return 1;
    }
    // Set socket to non-blocking
    u_long mode = 1;
    result = ioctlsocket(wSock, FIONBIO, &mode);
    if (result != 0){
        cout << "I/O Mode Error" << endl;
        closesocket(wSock);
        return 1;
    }
    // Connect to address
    result = WSAConnect(wSock, (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, NULL, NULL, NULL);
    if (result == SOCKET_ERROR){
        if (WSAGetLastError() != WSAEWOULDBLOCK){
            cout << "Connect Error" << ' ' << WSAGetLastError() << endl;
            closesocket(wSock);
            return 1;
        }

        // If no reply received,
        // Set timeout to timeout value
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(wSock, &fds);
        result = select(0, NULL, &fds, NULL, &tv);
        if (result <= 0){
            // cout << "Port " << port << " is not open" << endl;
            retvalue = 1;
        } else {
            //cout << "Port " << port << " is open" << endl;
            retvalue = 0;
            // Disconnect socket
            result = WSASendDisconnect(wSock, NULL);
            if (result != 0){
                cout << "Disconnect Error" << endl;
                closesocket(wSock);
                return 1;
            }
        }
    }
    closesocket(wSock);
    return retvalue;
}

void print_usage(){
    cout << "Usage: egress.exe [options]" << endl;
    cout << "OPTIONS:" << endl;
    cout << "  -h print help text" << endl;
    cout << "  -t <threads> set number of threads (default: 10)" << endl;
    cout << "  -p <startport>-<endport> choose port range (default: 1-1024)" << endl;
}