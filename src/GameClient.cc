#include <thread>
#include "Game.h"

int main(int argc, char **argv)
{
	if(argc < 4){
		std::cout << "Usage: <programa> <server adrr> <port addr> <nick-name>\n"; 
		return -1;
	}

    	GameClient ec(argv[1], argv[2], argv[3]);

    	//Inicio del thread
    	std::thread([&ec](){ ec.net_thread(); }).detach();

	//Mensaje de inicio de sesión del jugador
    	ec.login();

	//Comienza el thread de lectura de input
    	ec.input_thread();

	std::cout << "\nFIN\n";

	return 0;
}

