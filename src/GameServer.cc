#include "Game.h"

int main(int argc, char **argv)
{
    	GameServer es(argv[1], argv[2]);

	//Comienzo del bucle principal del server
    	es.update();

    	return 0;
}
