#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<bits/stdc++.h>
#include <thread>
#include<zlib.h>
using namespace std;

vector<string> split(string &request, string delim);
void handleClient(int client, string directory);

#endif
