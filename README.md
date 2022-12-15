# egress-analyzer
Lightweight, standalone, command-line based TCP egress analyzer for Windows used to test firewall rules. The executable operates as both a client to scan a server, or as a server to open specified ports.

Inspired by SEC530. Thanks to Ismael Valenzuela and Justin Henderson for the great course!

# Build
```
g++ main.cpp client.cpp server.cpp -o egress.exe -lws2_32 -static-libstdc++ -static -static-libgcc
```

# Example Usage
Testing a firewall allows port 80 but blocks other ports:
```
.\egress.exe --server -p 79-81
Opening ports (this may take a while)...
Done!
```
```
.\egress.exe -i 192.168.118.131 -p 79-81
Starting 10 thread(s)
Port 80 is open
Open ports: 1
Closed ports: 2
```
# Usage
```
> .\egress.exe -h
Usage: egress.exe [options]
OPTIONS:
  --server opens ports instead of connects to them
  -i <host> choose the host to scan
  -h print help text
  -t <threads> set number of threads (default: 10)
  -p <startport>-<endport> choose port range (default: 1-1024)
```
Scan all ports
```
.\egress.exe -p 1-65535
```
Scan ports 80 and 100-200 with 20 threads
```
.\egress.exe -p 80,100-200 -t 20
```
Scan allports.exposed
```
.\egress.exe -i allports.exposed
```
Open ports 1-1024 on a system
```
.\egress.exe --server -p 1-1024
```
