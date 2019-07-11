# Keysteal

For now it's small piece of software at mine beginning of writing FUD (full-undetectable) malware - keylogger for Windows written in C/C++.

## Getting Started

Download source from github

```
git clone https://github.com/Djkusik/Keysteal.git
```

To compile it, I used few flags for smaller size of final binary, faster processing time, stripping metadata or hiding binary from antiviruses:

```
i686-w64-mingw32-g++ -Os -s -std=c++11 keysteal.cpp -o keysteal.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
```

## Usage

For know it will only log buttons inside console window.

### Dislaimer

This software has been made for educational purposes only. I don't promote malicious practices and will not be responsible for any illegal activities. Use it at your own risk.