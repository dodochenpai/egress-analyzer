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

int open_port(int port);
void print_usage();

int main(int argc, char* argv[]) 
{
    vector<thread> threads;
    int numPorts=10;
    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    int portStart;
    int portEnd;

    // Convert command line arguments to strings
    vector<string> argList(argv, argv + argc);
    vector<int> ports;

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
        } else if (argList[i] == "-h"){
            print_usage();
            return 0;
        }
    }

    cout << "Opening ports (this may take a while)..." << endl;
    for (int i=0;i<ports.size();i++){
        threads.push_back(thread(open_port, ports[i]));
        Sleep(2);
    }
    cout << "Done!";
    for (int i=0;i<threads.size();i++){
        threads[i].join();
    }

    WSACleanup();

    return 0;
}

int open_port(int port){

    SOCKET ListenSocket;
    SOCKET ClientSocket;

    struct addrinfo *addrinfo = NULL;
    struct addrinfo hints;

    int iSendResult;
    int result;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char portbuffer[7];
    itoa(port, portbuffer, 10);

    result = getaddrinfo(NULL, portbuffer, &hints, &addrinfo);
    if (result != 0) {
        cout << "getaddrinfo failed" << endl;
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket failed" << endl;
        return 1;
    }

    // Setup the TCP listening socket
    result = bind( ListenSocket, addrinfo->ai_addr, (int)addrinfo->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(ListenSocket);
        return 1;
    }

    freeaddrinfo(addrinfo);

    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(ListenSocket);
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Accept failed" << endl;
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // shutdown the connection since we're done
    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed" << endl;
        closesocket(ClientSocket);
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    
    return 0;
}

void print_usage(){
    cout << "Usage: egressserver.exe [options]" << endl;
    cout << "OPTIONS:" << endl;
    cout << "  -h print help text" << endl;
    cout << "  -p <startport>-<endport> choose port range (default: 1-1024)" << endl;
}