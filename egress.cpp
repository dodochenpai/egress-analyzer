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

// Global Statistics
int globalOpenCount = 0;
int globalClosedCount = 0;

int main(int argc, char *argv[]){
    // Create default variable values
    vector<thread> threads;
    int numThreads = 10;
    int portStart = 0;
    int portEnd = 1024;    

    // Convert command line arguments to strings
    vector<string> argList(argv, argv + argc);

    // Parse Inputs
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
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup Failed" << endl;
        return 1;
    }

    // Create threads
    cout << "Starting " << numThreads << " thread(s)\n";
    for(int i=0;i<numThreads;i++){
        threads.push_back(thread(thread_handler, i+portStart, numThreads, portEnd));
    }

    // Wait for threads to finish before exiting
    for (int i=0;i<numThreads;i++){
        threads[i].join();
    }

    // Print out some statistics
    cout << "Open ports: " << globalOpenCount << endl;
    cout << "Closed ports: " << globalClosedCount << endl;
    //cout << "See outfile for more output" << endl;

    WSACleanup();
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
    SOCKET wSock;
    struct sockaddr_in sockaddr;
    struct timeval tv;
    int result;

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
        return 1;
    }
    // Set socket to non-blocking
    u_long mode = 1;
    result = ioctlsocket(wSock, FIONBIO, &mode);
    if (result != 0){
        cout << "I/O Mode Error" << endl;
        return 1;
    }
    // Connect to address
    result = WSAConnect(wSock, (SOCKADDR*)&sockaddr, sizeof(sockaddr), NULL, NULL, NULL, NULL);
    if (result == SOCKET_ERROR){
        if (WSAGetLastError() != WSAEWOULDBLOCK){
            cout << "Connect Error" << endl;
            return 1;
        }

        // If no reply received,
        // Set timeout to timeout value
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(wSock, &fds);
        result = select(0, NULL, &fds, NULL, &tv);
        if (result <= 0){
            //cout << "Port " << port << " is not open" << endl;
            globalClosedCount++;
        } else {
            //cout << "Port " << port << " is open" << endl;
            globalOpenCount++;
        }
    }
    // Disconnect socket
    result = WSASendDisconnect(wSock, NULL);
    if (result != 0){
        cout << "Disconnect Error" << endl;
        return 1;
    }
    return 0;
}

void print_usage(){
    cout << "Usage: egress.exe [options]" << endl;
    cout << "OPTIONS:" << endl;
    cout << "  -h print help text" << endl;
    cout << "  -t <threads> set number of threads (default: 10)" << endl;
    cout << "  -p <startport>-<endport> choose port range (default: 0-1024)" << endl;
}