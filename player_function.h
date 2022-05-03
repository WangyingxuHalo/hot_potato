#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "potato.h"
#include <vector>
#include <arpa/inet.h>
#include <algorithm>

#define MAX_DIGITS 5

using namespace std;

int init_client(const char *port, const char *hostname) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(host_info_list);

  return socket_fd;
}

// Create server part
int init_server() {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port = "";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  freeaddrinfo(host_info_list);
  return socket_fd;
}

int get_cur_server_port(int cur_server_socket_fd) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(cur_server_socket_fd, (struct sockaddr *)&sin, &len) == -1) {
        cerr << "Get port error" << endl;
    }
    return ntohs(sin.sin_port);
}

int init_accept(int socket_fd) {
    // Accept clients' request
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      exit(EXIT_FAILURE);
    }
    return client_connection_fd;
}

// Wait to receive potato from next server or previous client
void wait_receive_potato(int comm_next_server, int comm_from_prev_client, int ring_master_socket_fd, int cur_player_no, int prev_no, int next_no) {
    Potato potato;
    while (true) {
        //receive potato using select
        int max_fd = max(comm_next_server, max(comm_from_prev_client, ring_master_socket_fd));
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ring_master_socket_fd, &readfds);
        FD_SET(comm_next_server, &readfds);
        FD_SET(comm_from_prev_client, &readfds);
        select(max_fd+1, &readfds, NULL, NULL, NULL);
        int res;
        
        if (FD_ISSET(ring_master_socket_fd, &readfds)) {
        res = recv(ring_master_socket_fd, &potato, sizeof(potato), MSG_WAITALL);
        }
        if (FD_ISSET(comm_next_server, &readfds)) {
        res = recv(comm_next_server, &potato, sizeof(potato), MSG_WAITALL);
        }
        if (FD_ISSET(comm_from_prev_client, &readfds)) {
        res = recv(comm_from_prev_client, &potato, sizeof(potato), MSG_WAITALL);
        }

        if (potato.get_hops_left() == 0 || res == 0) {
          break;
        } else if (potato.get_hops_left() >= 1) {
            potato.hops_minus_one();
            potato.num_pass_plus_one();
            potato.trace_add_this(cur_player_no);
            if (potato.get_hops_left() == 0) {
              cout << "I'm it" << endl;
              send(ring_master_socket_fd, &potato, sizeof(potato), 0);
            } else {
              srand((unsigned int)time(NULL) + potato.get_hops_left());
              int random = rand() % 2;
              // Send to previous client
              if (random == 0) {
                  send(comm_from_prev_client, &potato, sizeof(potato), 0);
                  cout << "Sending potato to " << prev_no << endl;
              } else {
                // Send to next server
                  send(comm_next_server, &potato, sizeof(potato), 0);
                  cout << "Sending potato to " << next_no << endl;
              }
            }
        }
    }
}

void shut_down_client(int ring_master_socket_fd, int comm_next_server, int comm_from_prev_client) {
    close(ring_master_socket_fd);
    close(comm_next_server);
    close(comm_from_prev_client);
    close(ring_master_socket_fd);
}