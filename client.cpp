#include "client.h"

int startClient(vector<int> ports, int numThreads, string host){

    // Create default variable values
    vector<future<int>> futures;

    // Initialize WinSock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup Failed" << endl;
        return 1;
    }

    // Get Hostname Information
    string ipAddress;
    hostent * host_info = gethostbyname(host.c_str());
    if (host_info == NULL){
        cout << "Failed to resolve host" << endl;
        return 0;
    } else {
        struct in_addr addr;
        addr.s_addr = *(u_long *) host_info->h_addr_list[0];
        ipAddress = inet_ntoa(addr); 
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

    // Create threads
    cout << "Starting " << numThreads << " thread(s)\n";
    for(int i=0;i<numThreads;i++){
        // threads.push_back(thread(thread_handler, i+portStart, numThreads, portEnd));
        futures.push_back(async(thread_handler, threadTasks[i], ipAddress));
    }

    // Wait for threads to finish before exiting
    for (int i=0;i<numThreads;i++){
        //threads[i].join();
        futures[i].wait();
    }
    // Calculate number of open ports
    int openCount = 0;
    for (int i=0; i<futures.size();i++){
        openCount += futures[i].get();
    }
    int closedCount = ports.size() - openCount;
    // Print out some statistics
    cout << "Open ports: " << openCount << endl;
    cout << "Closed ports: " << closedCount << endl;
    // cout << "See outfile for more output" << endl;

    WSACleanup();
    return 0;

}

// Calculates the ports each thread will scan
// Returns total number of open ports
int thread_handler(vector<int> ports, string ipAddress){
    // Set variable to initial value
    int openCount = 0;
    int closedCount = 0;
    int retvalue;
    for (int i=0;i<ports.size();i++){
        // Scan the port
        retvalue = test_port(ports[i], ipAddress);
        if (retvalue == 0){
            openCount++;
        }
    }
    return openCount;
}

int test_port(int port, string ipAddress){
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
    // string target = "45.79.204.144";
    // Localhost test
    // string target = "127.0.0.1";
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
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
            cout << "Port " << port << " is open" << endl;
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
    cout << "  --server opens ports instead of connects to them" << endl;
    cout << "  -i <host> choose the host to scan" << endl;
    cout << "  -h print help text" << endl;
    cout << "  -t <threads> set number of threads (default: 10)" << endl;
    cout << "  -p <startport>-<endport> choose port range (default: 1-1024)" << endl;
}