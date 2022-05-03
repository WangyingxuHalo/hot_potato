# hot potato project
For this project, I use c++ to implement a game called Hot Potato. <br>

In this project, I use TCP sockets as the mechanism for communication between server and player application. <br>

## How does this game work?
1) Firstly, server part will define the port, the number of players playing this game as well as the number of times passing potatoes. <br>
2) Then, each player will connect to server. <br>
3) Server will pass a potato to a random player. <br>
4) Then this potato will either pass to previous player or next player until the passing times reaches what defined before. <br>
5) In the end, server part will print the trace that potato passes and the last player receiving potato will print information. <br>

## How to play this game?
1) For server part, the command should be: <br>
ringmaster <port_num> <num_players> <num_hops> <br> 
(example: ./ringmaster 1234 3 100) <br>

2) For player part, the command should be: <br>
player <machine_name> <port_num> <br>
(example: ./player vcm-xxxx.vm.duke.edu 1234)
