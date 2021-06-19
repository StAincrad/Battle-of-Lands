#include "Game.h"

//Contador de clientes
std::condition_variable num_cv;
std::mutex num_mutex;
int num_clientes = 0;

//----------Clase Rol---------//
Rol::Rol() : 
	type(MessageType::NONE),
	vida(0),
	mana(0),
	atk(0),
	mana_r(0),
	nick("none"),
	rol_t(RolType::NONE),
	command("none") {}

Rol::Rol(Player * p, const MessageType& t) : type(t)
{
	//Inicialización de variables en función del player
	vida = p->getVida();
	mana = p->getMana();
	atk = p->getAtk();
	mana_r = p->getManaR();
	nick = p->getNick();
	rol_t = p->getRol();
	command = "none";
}

void Rol::to_bin()
{
	alloc_data(ROL_SIZE);
	memset(_data, 0, ROL_SIZE);
	
	//Puntero auxiliar
	char* tmp = _data;

	//Alojamiento del tipo de mensaje
	memcpy(tmp, &type, sizeof(uint8_t));
	tmp += sizeof(uint8_t);

	//Alojamiento de RolType
	memcpy(tmp, &rol_t, sizeof(uint8_t));
	tmp += sizeof(uint8_t);
	
	//Alojamiento del nick
	memcpy(tmp, nick.c_str(), sizeof(char) * NICK_S);
	tmp += sizeof(char) * NICK_S;
	
	//Alojamiento del comando
	memcpy(tmp, command.c_str(), sizeof(char) * COM_SIZE);
	tmp += sizeof(char) * COM_SIZE;

	//Alojamiento de los stats restantes
	memcpy(tmp, &vida, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(tmp, &mana, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(tmp, &atk, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(tmp, &mana_r, sizeof(int));
}

int Rol::from_bin(char* bobj)
{
	alloc_data(ROL_SIZE);
	memcpy(static_cast<void*>(_data), bobj, ROL_SIZE);
	
	char* tmp = _data;

	//Tipo de mensaje
	memcpy(&type, tmp, sizeof(uint8_t));
	tmp += sizeof(uint8_t);

	//Tipo de Rol
	memcpy(&rol_t, tmp, sizeof(uint8_t));
	tmp += sizeof(uint8_t);
		
	//Nick de usuario
	nick = tmp;
	tmp += sizeof(char) * NICK_S;

	//Comando
	command = tmp;
	tmp += sizeof(char) * COM_SIZE;

	//Stats
	memcpy(&vida, tmp, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(&mana, tmp, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(&atk, tmp, sizeof(int));
	tmp += sizeof(int);
	
	memcpy(&mana_r, tmp, sizeof(int));
	
	return 0;	
}

//Get // SET
std::string Rol::getNick() const
{
	return nick;
}

MessageType Rol::getMsgType() const
{
	return type;
}

RolType Rol::getRol() const
{
	return rol_t;
}

int Rol::getVida() const
{
	return vida;
}

int Rol::getMana() const
{
	return mana;
}

int Rol::getAtk() const
{
	return atk;
}

int Rol::getManaR() const
{
	return mana_r;
}


void Rol::setNick(const std::string& n)
{
	nick = n;
}

void Rol::setCommand(const std::string& c)
{
	command = c;
}

void Rol::setMsgType(const MessageType& newType)
{
	type = newType;
}

//----------Clase-GameMessage------------// 

GameMessage::GameMessage(){}

GameMessage::GameMessage(const std::string& m) : message(m) {}

void GameMessage::to_bin()
{
    	alloc_data(MESSAGE_SIZE);

    	memset(_data, 0, MESSAGE_SIZE);

	char* tmp = _data;
       	//Alojamiento del tipo
	memcpy(tmp, &type, sizeof(MessageType));
	tmp += sizeof(MessageType);

	//Alojamiento del mensaje
	memcpy(tmp, message.c_str(), sizeof(char) * SERVER_MSG_SIZE);
}

int GameMessage::from_bin(char * bobj)
{
    	alloc_data(MESSAGE_SIZE);

    	memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

	char* tmp = _data;

	//Tipo de mensaje
	memcpy(&type, tmp, sizeof(MessageType));
	tmp += sizeof(MessageType);

	//Mensaje
	message = tmp;

    	return 0;
}

//-----------Clase-GameServer----------//
GameServer::GameServer(const char* s, const char* p) : socket(s, p) 
{
	//Creación del mensaje de Bienvenida
	welcome = "¡Bienvenido a Battle of Lands!\n" +
		std::string("Por favor, escoja un rol para luchar por su tierra\n") +
		std::string("Pulse 'G' para escoger Guerrero\n") +
		std::string("Pulse 'M' para escoger Mago\nPulse 'A' para escoger Asesino\n\0");
}

void GameServer::update()
{
	if(socket.bind() == -1){
		std::cerr << "Fallo en la unión (BIND) del socket\n";
		return;
	}
	
	//bucle ppal	
    	while (true)
    	{
        	/*
         	* NOTA: los clientes están definidos con "smart pointers", es necesario
         	* crear un unique_ptr con el objeto socket recibido y usar std::move
         	* para añadirlo al vector
         	*/

        	//Recibir Mensajes en y en función del tipo de mensaje
        	// - LOGIN: Añadir al vector clients
        	// - LOGOUT: Eliminar del vector clients
        	// - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
    	
		Rol rol;
		Socket* s = nullptr;

		std::cout << "Esperando mensajes\n";
		
		int r = socket.recv(rol, s);
		if(r == -1){
			std::cout << "Error al recibir mensaje\n";
		}
		
		std::cout << "MSG  TYPE: " << (int)(rol.getMsgType()) << "\n";
		switch(rol.getMsgType()){
		case MessageType::LOGIN:
		{
			//Se añade el nuevo socket al vector de clientes
			std::cout << rol.getNick() << " ha entrado en el juego\n";
			//clients.push_back(std::move(std::make_unique<Socket>(*s)));
		
			if(s == nullptr) std::cout << "NULLPTR\n";

			clients[num_clientes] = std::move(std::make_unique<Socket>(*s));
			std::cout << "PUSH BACK \n";
			
			//Si es el que se acaba de conectar
			//se le envía el mensaje de bienvenida
			std::cout << "DECLARACIÓN\n";
			GameMessage msg;
			std::cout << "MSG = \n";
			msg.message = welcome;
			std::cout << "MSG T = \n";
			msg.type = MessageType::LOGIN;
			std::cout << "LLEGA A SEND\n";
			socket.send(msg, *clients[num_clientes]);
		
			num_clientes++;
			
			//Se le pregunta por el Rol
			//Cuando responda, el cliente enviará un mensaje de tipo ROLED
			//para informa de que ha escogido el rol	
			break;
		}
		case MessageType::LOGOUT:
			break;
		case MessageType::FINISH_ROUND:
			break;
		case MessageType::FINISH_GAME:
			std::cout << "ROL 3\n";
			break;
		case MessageType::ROLED:
			break;
		case MessageType::COMMAND:
			break;
		}
	}
}

//--------Clase-Player-----------------//

Player::Player(const char* s, const char * p, const char * n) :
       	socket(s, p),
       	nick(n),
	vida(0),
	mana(0),
	atk(0),
	mana_r(0),
	type(RolType::NONE) {}

void Player::command()
{
}

void Player::login()
{
	Rol rol = Rol(this, MessageType::LOGIN);

	//Se le avisa al server de la conexión
    	int send = socket.send(rol, socket);
	if(send == -1){
		std::cerr << "Error en LOGIN\n";
	}
}

void Player::logout()
{
	//Funciona igual que el LOGIN
	std::string c;
	//rol.setMsgType(MessageType::LOGOUT);
	//rol.setCommand(c);
	//socket.send(rol, socket);
}

void Player::setRolType(RolType rt)
{
	type = rt;
	switch(type){
	case RolType::MAGO:
		vida = M_VIDA;
		mana = M_MANA;
		atk = M_ATK;
		mana_r = M_MR;
		break;
	case RolType::GUERRERO:
		vida = G_VIDA;
		mana = G_MANA;
		atk = G_ATK;
		mana_r = G_MR;
		break;
	case RolType::ASESINO:
		vida = A_VIDA;
		mana = A_MANA;
		atk = A_ATK;
		mana_r = A_MR;
		break;
	}
}

void Player::addVida(int v)
{
	vida += v;
}

void Player::addMana() 
{
	mana += mana_r;
}

std::string Player::getNick() const
{
	return nick;
}

RolType Player::getRol() const
{
	return type;
}

int Player::getVida() const
{
	return vida;
}

int Player::getMana() const
{
	return mana;
}

int Player::getAtk() const
{
	return atk;
}

int Player::getManaR() const
{
	return mana_r;
}

Socket* Player::getSocket() 
{
	return &socket;
}
//--------Clase-GameClient-------------//

GameClient::GameClient(const char* s, const char* p, const char* n) 
{
	//Creación del player en el juego del cliente
	player = new Player(s, p, n);
}

GameClient::~GameClient(){
	delete player;
}

void GameClient::login(){
	player->login();
}

void GameClient::input_thread()
{
	while (true)
    	{

        	// Leer stdin con std::getline
        	// Enviar al servidor usando socket
    	}
}

void GameClient::net_thread()
{
    	while(true)
    	{
		GameMessage m;
		player->getSocket()->recv(m);
	
		switch(m.type){	
		case MessageType::LOGIN:
			std::cout << "\n" << m.message << "\n";
			break;
		}
    	}
}

