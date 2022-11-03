# egress-analyzer
Lightweight, independent TCP egress analyzer for Windows used to identify what destination ports are or aren't blocked by a firewall.

# Usage
```
> .\egress.exe -h
Usage: egress.exe [options]
OPTIONS:
  -h print help text
  -t <threads> set number of threads (default: 10)
  -p <startport>-<endport> choose port range (default: 0-1024)
```
Scan all ports
```
> .\egress.exe -p 0-65535
```
Scan ports 0-1024 with 20 threads
```
> .\egress.exe -t 20
```
