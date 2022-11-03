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

WSADATA wsaData;
SOCKET wSock;
struct sockaddr_in sockaddr;

int test_port(int port);
int thread_handler(int initial, int numThreads, int max);
void print_usage();

using namespace std;

int globalOpenCount = 0;
int globalClosedCount = 0;

int main(int argc, char *argv[]){
    // Create default variable values
    vector<thread> threads;
    int numThreads = 10;
    int portStart = 0;
    int portEnd = 1024;    

    vector<string> argList(argv, argv + argc);

    for (int i=0;i<argList.size();i++){
        if (argList[i] == "-p"){
            string port = argList[i+1];
            for (int j=0;j<port.length();j++){
                if (port[j] == '-'){
                    portStart = stoi(port.substr(0, j));
                    portEnd = stoi(port.substr(j+1, port.length()-j));
                    break;
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
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create threads
    cout << "Starting " << numThreads << " thread(s)\n";
    for(int i=portStart;i<numThreads;i++){
        threads.push_back(thread(thread_handler, i, numThreads, portEnd));
    }

    // Wait for threads to finish before exiting
    for (int i=0;i<numThreads;i++){
        threads[i].join();
    }

    cout << "Open ports: " << globalOpenCount << endl;
    cout << "Closed ports: " << globalClosedCount << endl;
    cout << "See outfile for more output" << endl;

    return 0;

}

// Calculates the ports each thread will scan
int thread_handler(int initial, int numThreads, int max){
    // Set variable to initial value
    int i = initial;
    while (i<=max){
        // Scan the port
        test_port(i);
        // Set next port to scan
        i += numThreads;
    }
    return 0;
}

int test_port(int port){
    // Initialize Variables
    WSADATA wsaData;
    SOCKET wSock;
    struct sockaddr_in sockaddr;
    struct timeval tv;
    
    // Set Options
    //string target = "8.8.8.8";
    string target = "45.79.204.144";
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(target.c_str());
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // Create Socket
    wSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    // Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(wSock, FIONBIO, &mode);
    // Connect to address
    int result = WSAConnect(wSock, (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, NULL, NULL, NULL);
    if (result == SOCKET_ERROR){
        if (WSAGetLastError() != WSAEWOULDBLOCK){
            return false;
        }

        // If no reply received,
        // Set timeout to timeout value
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(wSock, &fds);
        result = select(0, NULL, &fds, NULL, &tv);
        if (result <= 0){
            cout << "Port " << port << " is not open" << endl;
            globalClosedCount++;
        } else {
            cout << "Port " << port << " is open" << endl;
            globalOpenCount++;
        }
    }
    // Disconnect socket
    WSASendDisconnect(wSock, NULL);
    return 0;
}

void print_usage(){
    cout << "Usage: egress.exe [options]" << endl;
    cout << "OPTIONS:" << endl;
    cout << "  -h print help text" << endl;
    cout << "  -t <threads> set number of threads" << endl;
    cout << "  -p <startport>-<endport> choose port range" << endl;
}