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
#include "server.h"
#include "client.h"

#define WIN32_LEAN_AND_MEAN

using namespace std;

WSADATA wsaData;
SOCKET wSock;
struct sockaddr_in sockaddr;

int main(int argc, char *argv[]){

    vector<int> ports;
    int portStart;
    int portEnd;
    // Convert command line arguments to strings
    vector<string> argList(argv, argv + argc);
    string host = "allports.exposed";
    int numThreads = 10;
    int client = 0;

    // Parse Inputs
    for (int i=0;i<argList.size()-1;i++){
        if (argList[i] == "--server"){
            client = 1;
        } else if (argList[i] == "-p"){
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
        } else if (argList[i] == "-h" | argList[i+1] == "-h"){
            print_usage();
            return 0;
        } else if (argList[i] == "-i"){
            host = argList[i+1];
        }
    }
    if (client == 1){
        startServer(ports);
    } else {
        startClient(ports, numThreads, host);
    }

    return 0;

}