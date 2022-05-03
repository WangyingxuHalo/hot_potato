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

using namespace std;

// Create server part
int init_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;

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

// Connect before starting game
void init_players(int socket_fd, const char *port_num, int num_players, vector<int> &fds, vector<int> &ports, vector<string> &addresses) {
  for (int i = 0; i < num_players; i++) {
    int player_server_port;
    string player_address;
    // Accept clients' request
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      exit(EXIT_FAILURE);
    }
    // Get port information
    player_address = inet_ntoa(((struct sockaddr_in *)&socket_addr)->sin_addr);

    // Put all the information into the vector
    fds.push_back(client_connection_fd);
    addresses.push_back(player_address);

    // Send to player their information and receive port of that player
    send(client_connection_fd, &i, sizeof(i), 0);
    send(client_connection_fd, &num_players, sizeof(num_players), 0);
    recv(client_connection_fd, &player_server_port, sizeof(player_server_port), 0);
    ports.push_back(player_server_port);

    // That player is ready to start game
    cout << "Player " << i <<  " is ready to play" << endl;
  }
}

// Send next server information(port & address) to each player to connect them together
void send_next_server_info(int num_players, vector<int> fds, vector<int> ports,vector<string> addresses) {
  for (int i = 0; i < num_players; i++) {
    int next_i = ( i + 1 ) % num_players;
    int next_port = ports[next_i];
    string next_address = addresses[next_i];
    char next_address_array[100];
    memset(next_address_array, 0, sizeof(next_address_array));
    strcpy(next_address_array, next_address.c_str());
    send(fds[i], &next_port, sizeof(next_port), 0);
    send(fds[i], &next_address_array, sizeof(next_address_array), 0);
  }
}

// Send potato to first chosen player
void send_potato(int num_players, vector<int> fds, Potato &potato) {
  srand((unsigned int)time(NULL) + num_players);
  int random = rand() % num_players;
  cout << "Ready to start the game, sending potato to player " << random << endl;
  send(fds[random], &potato, sizeof(potato), 0);
}

// Waiting to receive potato from the one who last receive potato
void receive_from_last_player(int num_players, vector<int> fds, Potato &potato) {
  fd_set readfds;
  FD_ZERO(&readfds);
  // Add each fd to set
  for (int i = 0 ; i < num_players; i++) {
    FD_SET(fds[i], &readfds);
  }
  // Start listening from different players
  int max_fd = *max_element(fds.begin(), fds.end());
  select(max_fd+1, &readfds, NULL, NULL, NULL);
  for (int i = 0 ; i < num_players; i++) {
    if (FD_ISSET(fds[i], &readfds)) {
      recv(fds[i], &potato, sizeof(potato), MSG_WAITALL);
      break;
  }
  }
}

// Close every fd
void shut_down_game(int num_players, vector<int> fds, int socket_fd) {
  for (int i = 0 ; i < num_players; i++) {
    close(fds[i]);
  }
  close(socket_fd);
}