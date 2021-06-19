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

class Player;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//Tipos de mensaje
enum class MessageType
{
	LOGIN   = 0,		//Inicio de sesión del jugador
        LOGOUT  = 1,		//Desconexión del jugador
	FINISH_ROUND = 2,	//Ronda terminada
	FINISH_GAME = 3,	//Juego acabado
	ROLED = 4, 		//Rol escogido
	COMMAND = 5,		//Mensaje de comando
	NONE = 6,		//Tipo vacío
};

//Tipos de rol
enum class RolType
{
	MAGO = 0,
	GUERRERO = 1,
	ASESINO = 2,
	NONE = 3	//Rol vacío
};

//-------CONSTANTES----------//
//MAX CLIENTS
#define MAX_CLIENTS 2

//NICK SIZE
#define NICK_S 10

//COMMAND SIZE 
#define COM_SIZE 5

//SERVER MESSAGE SIZE
#define SERVER_MSG_SIZE 300 

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
 * Clase de mensajería del rol elegido por el jugador.
 * Sirve para enviar toda la información que tiene guardada
 * el jugador.
 */
class Rol : public Serializable
{
public:
	//Tamaño maximo de rol
	static const size_t ROL_SIZE = sizeof(int) * 4 + sizeof(MessageType) + sizeof(char*) * COM_SIZE + sizeof(char) * NICK_S + sizeof(RolType);

	//Constructora por defecto
	Rol();
	//Constructora desde player
	Rol(Player * p, const MessageType& t);

	virtual void to_bin();
	virtual int from_bin(char* bobj);
	
	//GET-SET
	std::string getNick() const;
	MessageType getMsgType() const;
	RolType getRol() const;
	int getVida() const;
	int getMana() const;
	int getAtk() const;
	int getManaR() const;

	void setRolType(const RolType& rt);
	void setNick(const std::string& n);
	void setCommand(const std::string& c);
	void setMsgType(const MessageType& newType);
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
    	static const size_t MESSAGE_SIZE = sizeof(char) * (SERVER_MSG_SIZE) + sizeof(MessageType);


    	GameMessage();

    	GameMessage(const std::string& m);

    	void to_bin();

    	int from_bin(char * bobj);

    	MessageType type;

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
   	//Socket de los clientes
	std::unique_ptr<Socket> clients[MAX_CLIENTS];
	std::unique_ptr<Socket> client1;
	std::unique_ptr<Socket> client2;

    	/**
     	* Socket del servidor
     	*/
   	Socket socket;

	std::string welcome;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*
 * Clase para guardar toda la información del jugador
 * Así como para tratar el envío de mensajes
 */
class Player
{
public:
	Player(const char * s, const char * p, const char * n);
	
	/*
	 * Envía un mensaje de tipo COMMAND al server
	 */
	void command();
	
	/*
	 * Envía un mensaje de tipo LOGIN al server
	 */
	void login();
	

	/*
	 * Envía un mensaje de tipo LOGOUT al server
	 */
	void logout();

	//----------SET-----------//
	/*
	 * Asignación del rol
	 */
	void setRolType(RolType rt);
	
	/*
	 * Cambia la vida en función de v
	 */
	void addVida(int v);
	
	/*
	 * Cambia el maná en función de mana_r
	 */
	void addMana();
	
	//-----------GET--------------//
	std::string getNick() const;

	RolType getRol() const;
	
	int getVida() const;

	int getMana() const;

	int getAtk() const;

	int getManaR() const;

	Socket* getSocket();
private:
	Socket socket;
	std::string nick;

	//Stats del jugador
	int vida;	//Vida
	int mana;	//Mana
	int atk;	//Ataque
	int mana_r;	//Recuperación maná por ronda
	RolType type;	//Tipo de rol
};

/**
 *  Clase para el juego del cliente
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
	
	~GameClient();

	//Envía el mensaje de login
	void login();

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
	//Puntero a los datos del jugador
	Player* player;
};
	
