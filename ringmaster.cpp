#include "ringmaster_function.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "parameter number is not correct" << endl;
        return EXIT_FAILURE;
    }
    const char *port_num = argv[1];
    int num_players = atoi(argv[2]);
    if (num_players <= 1) {
        cerr << "number of player should be larger than 1" << endl;
        return EXIT_FAILURE;
    }
    int num_hops = atoi(argv[3]);
    if (num_hops < 0 || num_hops > 512) {
        cerr << "number of hops should be between 0 and 512" << endl;
        return EXIT_FAILURE;
    }
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << num_players << endl;
    cout << "Hops = " << num_hops << endl;

    // Initialize server part and start listening
    int socket_fd = init_server(port_num);

    // Information of players
    vector<int> fds;
    vector<int> ports;
    vector<string> addresses;
    
    // Accept each players' request
    init_players(socket_fd, port_num, num_players, fds, ports, addresses);
    
    // Send each player's next server information to them
    send_next_server_info(num_players, fds, ports, addresses);

    // Start passing potato
    Potato potato;
    potato.set_hop_num(num_hops);
    // If the number of hops is larger than 0 
    if (num_hops != 0) {
      send_potato(num_players, fds, potato);
      receive_from_last_player(num_players, fds, potato);
      cout << "Trace of potato:" << endl;
      potato.print_trace();
    }
    // If the number of hops is 0, directly shut down the game
    shut_down_game(num_players, fds, socket_fd);
    return EXIT_SUCCESS;
}
