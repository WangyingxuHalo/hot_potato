#include "player_function.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "parameter number is not correct" << endl;
        return EXIT_FAILURE;
    }
    const char *hostname = argv[1];
    const char *port = argv[2];

    // Get my player number and total player number
    int ring_master_socket_fd = init_client(port, hostname);
    int player_no;
    int player_num;
    recv(ring_master_socket_fd, &player_no, sizeof(player_no), 0);
    recv(ring_master_socket_fd, &player_num, sizeof(player_num), 0);
    int next_no = (player_no + 1) % player_num;
    int prev_no = (player_no + player_num - 1) % player_num;
    cout << "Connected as player " << player_no << " out of " << player_num <<" total players" << endl;
    
    // Server also has port number
    int cur_server_socket_fd = init_server();
    int server_port = get_cur_server_port(cur_server_socket_fd);
    // Tell ring master my port number
    send(ring_master_socket_fd, &server_port, sizeof(server_port), 0);

    // Get the next connected server information, put them in array
    int next_port;
    char next_addr[100];
    int len_port = recv(ring_master_socket_fd, &next_port, sizeof(next_port), 0);
    // cout << "len of port:" << len_port << endl;
    char port_char[MAX_DIGITS + sizeof(char)];
    // cout << "size of char: " << sizeof(char) << endl;
    sprintf(port_char, "%d", next_port);

    int len_addr = recv(ring_master_socket_fd, &next_addr, sizeof(next_addr), 0);
    // cout << "len of addr: " << len_addr << endl;

    // This is a client connecting to i + 1 server
    int comm_next_server = init_client(port_char, next_addr);

    // This is also a server waiting for connection from i - 1 client
    int comm_from_prev_client = init_accept(cur_server_socket_fd);
    wait_receive_potato(comm_next_server, comm_from_prev_client, ring_master_socket_fd, player_no, prev_no, next_no);
    shut_down_client(ring_master_socket_fd, comm_next_server, comm_from_prev_client);

    return EXIT_SUCCESS;
}