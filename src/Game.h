#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"
#include "Socket.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Tipos de rol
enum RolType{
	MAGO = 0,
	GUERRERRO = 1,
	ASESINO = 2
}
//-------CONSTANTES----------//
//MAX CLIENTS
#define MAX_CLIENTS 2

//NICK SIZE
#define NICK_S 10

//MESSAGE SIZE 
#define BUF_SIZE 5

//MAGO
#define M_VIDA 100
#define M_MANA 100
#define M_ATK 100
#define M_MR 10

//GUERRERO
#define G_VIDA 100
#define G_MANA 100
#define G_ATK 100
#define G_MR 10

//ASESINO
#define A_VIDA 100
#define A_MANA 100
#define A_ATK 100
#define A_MR 10
/*
 * Clase que guarda los valores del rol 
 * elegido por el jugador
 */
class Rol : public GameMessage{
public: 
	//Constructora
	Rol(uint8_t t, );
	
	virtual void to_bin();
	virtual void from_bin(char* data);

private:
	//Vida del personaje
	int vida = 0;
	//Mana del personaje
	int mana = 0;
	//Ataque del personaje
	int atk = 0;
	//Mana que recupera por turno
	int mana_r = 0;
	//Tipo del personaje
	uint8_t type;
};

/**
 *  Mensaje del protocolo del juego
 *
 *  +-------------------------+
 *  | Tipo: uint8_t           | 0 (login), 1 (mensaje), 2 (logout)
 *  +-------------------------+
 *  | Nick: char[NICK_S]      | Nick incluido el char terminación de cadena '\0'
 *  +-------------------------+
 *  |                         |
 *  | Mensaje: char[BUF_SIZE] | Mensaje incluido el char terminación de cadena '\0'
 *  |                         |
 *  +-------------------------+
 *  |                         |
 *  | Rol: int * 3 + uint8_t  | Rol del jugador
 *  |                         |
 *  +-------------------------+
 *
 */
class GameMessage: public Serializable
{
public:
    	static const size_t MESSAGE_SIZE = sizeof(char) * (BUF_SIZE + NICK_S) 
			 + sizeof(uint8_t);

    	enum MessageType
    	{
        	LOGIN   = 0,
        	MESSAGE = 1,
        	LOGOUT  = 2,
		FINISH_ROUND = 3,
		FINISH_GAME = 4
   	};

    	GameMessage(){};

    	GameMessage(const std::string& n, const std::string& m):nick(n),message(m){};

    	void to_bin();

    	int from_bin(char * bobj);

    	uint8_t type;

    	std::string nick;
    	std::string message;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el servidor de juego
 */
class GameServer
{
public:
    	GameServer(const char * s, const char * p): socket(s, p)
    	{
        	socket.bind();
    	};

    	/**
     	*  Thread principal del servidor recive mensajes en el socket y
     	*  lo distribuye a los clientes. Mantiene actualizada la lista de clientes
     	*/
    	void do_messages();

private:
    	/**
     	*  Lista de clientes conectados al servidor de Chat, representados por
     	*  su socket
     	*/
    	std::vector<std::unique_ptr<Socket>> clients;

    	/**
     	* Socket del servidor
     	*/
   	Socket socket;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el cliente de juego
 */
class GameClient
{
public:
    	 /**
     	 * @param s dirección del servidor
    	 * @param p puerto del servidor
    	 * @param n nick del usuario
    	 */
    	GameClient(const char * s, const char * p, const char * n):socket(s, p),
        	nick(n){};
    
	void comando();

    	/**
     	*  Envía el mensaje de login al servidor
     	*/
    	void login();

    	/**
     	*  Envía el mensaje de logout al servidor
     	*/
    	void logout();

    	/**
     	*  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     	*  y los envía por red vía el Socket.
     	*/
    	void input_thread();

    	/**
     	*  Rutina del thread de Red. Recibe datos de la red y los "renderiza"
     	*  en STDOUT
    	*/
    	void net_thread();

private:

    	/**
     	* Socket para comunicar con el servidor
     	*/
    	Socket socket;

    	/**
     	* Nick del usuario
     	*/
    	std::string nick;
	uint8_t type;
};

