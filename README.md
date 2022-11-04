# egress-analyzer
Lightweight, standalone, command-line based TCP egress analyzer for Windows used to identify the destination ports blocked by a firewall. The client can also be used within a local network by pointing to a device running the egress server.

# Build
Egress Client
```
> g++ egress.cpp -o egress.exe -lws2_32 -static-libstdc++ -static -static-libgcc
```
Egress Server
```
> g++ egressserver.cpp -o egressserver.exe -lws2_32 -static-libstdc++ -static -static-libgcc
```

# Usage
```
> .\egress.exe -h
Usage: egress.exe [options]
OPTIONS:
  -h print help text
  -t <threads> set number of threads (default: 10)
  -p <startport>-<endport>[,<single port>] choose port range (default: 1-1024)
```
Scan all ports
```
> .\egress.exe -p 1-65535
```
Scan ports 80 and 100-200 with 20 threads
```
> .\egress.exe -p 80,100-200 -t 20
```
Open ports 1-1024 on a system
```
> .\egressserver.exe
```
