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
	INIT_BATTLE = 2,	//Pasa al estado de batalla
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

enum class GameState
{
	CHOOSEN = 0, 	//Escogiendo el rol
	BATTLE = 1,	//Escogiendo siguiente ataque
	WAITING = 2,	//Esperando otro jugador
	FINISH = 3,	//Partida acabada

	NONE = 4	
};

//-------CONSTANTES----------//
//MAX CLIENTS
#define MAX_CLIENTS 2

//NICK SIZE
#define NICK_S 10

//COMMAND SIZE 
#define COM_SIZE 5

//SERVER MESSAGE SIZE
#define SERVER_MSG_SIZE 500

/*
 *	VIDA -> Vida inicial 
 *	MANA -> Mana inicial
 *	ATK -> Ataque 
 *	MR -> Mana restaurado por ronda
 */
 
//MAGO
#define M_VIDA 400
#define M_MANA 8
#define M_ATK 40
#define M_MR 3

//GUERRERO
#define G_VIDA 1300
#define G_MANA 4
#define G_ATK 15
#define G_MR 1

//ASESINO
#define A_VIDA 700
#define A_MANA 6
#define A_ATK 25
#define A_MR 2

/*
 * Clase de mensajería del rol elegido por el jugador.
 * Sirve para enviar toda la información que tiene guardada
 * el jugador.
 */
class Rol : public Serializable
{
public:
	//Tamaño maximo de rol
	static const size_t ROL_SIZE = sizeof(int) * 5 + sizeof(MessageType) + sizeof(char*) * COM_SIZE + sizeof(char) * NICK_S + sizeof(RolType);

	//Constructora por defecto
	Rol();
	//Constructora desde player
	Rol(Player * p, const MessageType& t);

	virtual void to_bin();
	virtual int from_bin(char* bobj);
	
	//GET-SET
	std::string getNick() const;
	std::string getCommand() const;
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
	
	// Cambia la vida en función de v
	void addVida(int v);
	
	//Cambia el maná en función de m
	void addMana(int m);

	//Restaura el maná al acabar la ronda en función de mana_r
	void reloadMana();
private:
	//Vida del personaje
	int vida;
	//Mana del personaje
	int mana;
	//Mana maximo del personake
	int manaMax;
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

	//Serializa los stats
	void seriStats(char* tmp);
	//Deserializa los stats
	void deseriStats(char* tmp);
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
    	//Socket del server
   	Socket socket;
	
	//Datos de los clientes
	struct ClientSocket
	{
		Rol stats;
		std::unique_ptr<Socket> socket;
		bool used;
	};
   	//Socket de los clientes
	ClientSocket clients[MAX_CLIENTS];

	//Determina el número de jugadores que han escogido rol
	int roledPlayers;
	//Numero de clientes actual
	int num_clientes;
	//Posicion del enemigo en el array de clientes
	int enemigo;
	//Numero de clientes que ya han mandado el comando: va de 0 a 3
	//(0 = ningun comando enviado; 1 = un comando enviado;
	//2 = dos comandos enviados; 3 = se ha procesado ya uno de los 
	//comandos enviados)
	int num_commands = 0;
	//Porcentaje del daño mitigado
	float m_dmg = 0.3;

	//Mensaje de bienvenida
	std::string welcome;
	//Mensaje con los comandos del juego
	std::string commands;
	//Guarda el primer comando enviado en el estado de combate
	std::string reroled;
	//Array con los nicks de los jugadores
	//para evitar coger el mismo
	std::string nicks[MAX_CLIENTS];
	
	//--------METODOS-PRIVADOS------------//
	
	//Gestiona los mensajes de LOGIN
	void manageLogin(const Rol& rol, Socket* s);

	//Gestiona los mensajes de ROLED
	void manageRoled(const Rol& rol, Socket* s);
	
	//Gestiona los mensajes de LOGOUT
	void manageLogout(const Rol& rol, Socket* s);

	//Gestiona los comandos
	void manageCommand(const Rol& rol, Socket* s);

	//Gestiona el reincio del juegi
	void manageRestart(const Rol& rol, Socket* s);

	//Gestiona la fase de combate en función de los comandos de los juagadores
	//current -> jugador al que se le procesa el comando
	//enemigo -> el otro jugador
	void manageBattle(const int& current, const int& enemigo);

	//Comprueba el estado de las vidas de los jugadores tras el combate
	bool isFinished(const int& current, const int& enemigo);

	//Devuelve el string del rol
	std::string printRol(const RolType& t);

	//Devuelve el informe de batalla
	std::string informeEnemigo(const int& i);
	std::string informe(const int& i);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*
 * Clase para guardar toda la información inicial del jugador.
 * Sirve para escoger el rol inicial y enviarselo al servidor 
 * para que guarde los stats.
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
	//Controla el bucle de net_thread
	bool exit;

	//Puntero a los datos del jugador
	Player* player;

	//Administra el estado del juego en el cliente
	GameState state;
	
	//Procesa el comando para escoger rol
	void chooseRol(const std::string& msg);
	//Procesa el comando para elegir una acción de batalla
	void chooseAction(const std::string& msg);
	//Procesa el comando cuando acaba la partida
	void chooseFinish(const std::string& msg);
};
	
