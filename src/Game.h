#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

//MUTEX Y CONDITION para evitar DDoS
#include <mutex>
#include <condition_variable>

#include "Serializable.h"
#include "Socket.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//Tipos de mensaje
enum class MessageType
{
	LOGIN   = 0,		//Inicio de sesión del jugador
        LOGOUT  = 1,		//Desconexión del jugador
	FINISH_ROUND = 2,	//Ronda terminada
	FINISH_GAME = 4,	//Juego acabado
	ROLED = 5, 		//Rol escogido
	COMMAND = 6		//Mensaje de comando
};

//Tipos de rol
enum class RolType
{
	MAGO = 0,
	GUERRERO = 1,
	ASESINO = 2
};

//-------CONSTANTES----------//
//MAX CLIENTS
#define MAX_CLIENTS 2

//Contador de clientes
std::condition_variable num_cv;
std::mutex num_mutex;
int num_clientes = 0;

//NICK SIZE
#define NICK_S 10

//MESSAGE SIZE 
#define BUF_SIZE 5

/*
 *	VIDA -> Vida inicial 
 *	MANA -> Mana inicial
 *	ATK -> Ataque 
 *	MR -> Mana restaurado por ronda
 */
 
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

/**
 *  Estructura de la serialización del jugador
 *
 *  +-------------------------+
 *  | Tipo: uint8_t           | Tipo del mensaje
 *  +-------------------------+
 *  | Nick: char[NICK_S]      | Nick incluido el char terminación de cadena '\0'
 *  +-------------------------+
 *  |                         |
 *  | Mensaje: char[BUF_SIZE] | Mensaje enviado al servidor '\0'
 *  |                         |
 *  +-------------------------+
 *  |                         |
 *  | Rol: int * 3 + uint8_t  | Rol del jugador
 *  |                         |
 *  +-------------------------+
 *
 */

/*
 * Clase que guarda los valores del rol 
 * elegido por el jugador
 */
class Rol : public Serializable
{
public:
	//Tamaño maximo de rol
	static const size_t ROL_SIZE = sizeof(int) * 4 + sizeof(uint8_t) * 2 + sizeof(char*) * (BUF_SIZE + NICK_S);

	//Constructora
	Rol();
	~Rol();

	virtual void to_bin();
	virtual int from_bin(char* data);
	
	//GET-SET
	std::string getNick() const;
	MessageType getMsgType() const;
	RolType getRol() const;
	int getVida() const;
	int getMana() const;
	int getAtk() const;
	int getManaR() const;

	void setRolType(RolType rt);
	void setNick(std::string n);
	void setCommand(std::string c);
	void setMsgType(MessageType newType);
private:
	//Vida del personaje
	int vida;
	//Mana del personaje
	int mana;
	//Ataque del personaje
	int atk;
	//Mana que recupera por turno
	int mana_r;
	//Tipo del personaje
	RolType rol_t;
	//Tipo de mensaje
	MessageType type;
	
	std::string nick;
	std::string command;
};

class GameMessage: public Serializable
{
public:
	//Tamaño máximo del mensaje serializable
    	static const size_t MESSAGE_SIZE = sizeof(char) * (BUF_SIZE + NICK_S) + sizeof(uint8_t);


    	GameMessage(){};

    	GameMessage(const std::string& n, const std::string& m):nick(n),message(m){};

    	void to_bin();

    	int from_bin(char * bobj);

    	MessageType type;

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
    	GameServer(const char * s, const char * p);

	/* Thread principal del server. Envía mensajes al resto
 	* de usuarios y mantiene actualizada la lista
	* de jugadores. 	
	*/ 
    	void update();

private:
   	//Listad de los sockets de los clientes
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
    	GameClient(const char * s, const char * p, const char * n);
   
 	//Envía un mensaje de tipo comando al servidor
	void command();

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
	Rol rol;	
};
	
