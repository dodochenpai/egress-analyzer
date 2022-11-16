#include "server.h"

extern WSADATA wsaData;
extern SOCKET wSock;
extern struct sockaddr_in sockaddr;

int startServer(vector<int> ports){
    vector<thread> threads;
    int numPorts=10;
    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
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