#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <future>

using namespace std;

extern WSADATA wsaData;
extern SOCKET wSock;
extern struct sockaddr_in sockaddr;

int test_port(int port, string ipAddress);
int thread_handler(vector<int> ports, string ipAddress);
void print_usage();
int startClient(vector<int> ports, int threads, string host);