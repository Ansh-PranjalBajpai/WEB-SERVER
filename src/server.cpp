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
#include "client_handler.h"
using namespace std;


int main(int argc, char **argv)
{
  cout << unitbuf;
  cerr << unitbuf;

  string directory;
  for (int i=1;i<argc;i++)
  {
    if (strcmp(argv[i],"--directory")==0&&i+1<argc)
    {
      directory  = argv[i+1];
      break;
    }
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    cerr << "Failed to create server socket\n";
    return 1;
  }

  ssize_t reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
  {
    cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    cerr << "listen failed\n";
    return 1;
  }

  while(true)
  {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    cout << "Waiting for a client to connect...\n";

    int client = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
     thread(handleClient, client,directory).detach();
  }
  

  close(server_fd);

  return 0;
}
