all: player ringmaster

player: player.cpp player_function.h
	g++ -o player player.cpp

ringmaster: ringmaster.cpp ringmaster_function.h
	g++ -o ringmaster ringmaster.cpp

clean:
	rm -f player ringmaster