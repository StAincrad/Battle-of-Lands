#include "Game.h"

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
GameServer::GameServer(const char* s, const char* p) :
       	socket(s, p),
       	roledPlayers(0),
	num_clientes(0)
{
	//Creación del mensaje de Bienvenida
	welcome = "¡Bienvenido a Battle of Lands!\n" +
		std::string("Por favor, escoja un rol para luchar por su tierra\n") +
		std::string("Pulse 'G' para escoger Guerrero\n") +
		std::string("Pulse 'M' para escoger Mago\n") +
		std::string("Pulse 'A' para escoger Asesino\n\0");
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
		 * En el server siempre se recibe un mensaje con características de rol,
		 * de manera que en función del tipo de mensaje que contenga se harán unas
		 * cosas u otras. 
		 * LOGIN: Añade nuevos jugadores en caso de que haya hueco. Tras esto, se les envía
		 * el mensaje de bienvenida para que escojan su rol.
		 * LOGOUT: Elimina al jugador que se haya desconectado
		 * ROLED: Cuando el jugador escoja rol, si no todos han escogido, entonces simplemente
		 * se le avisa al que haya elegido para que espere. Cuando los dos hayan elegido, se 
		 * procede al gamestate de MAINLOOP.
		 */
		Rol rol;
		Socket* s = nullptr;

		std::cout << "Esperando mensajes\n";
		
		int r = socket.recv(rol, s);
		if(r == -1){
			std::cout << "Error al recibir mensaje\n";
		}
		
		switch(rol.getMsgType()){
		case MessageType::LOGIN:
			if(num_clientes >= MAX_CLIENTS) break;
			
			manageLogin(rol, s);
			break;
		case MessageType::LOGOUT:
			num_clientes--;
			break;
		case MessageType::FINISH_ROUND:
			break;
		case MessageType::FINISH_GAME:
			break;
		case MessageType::ROLED:
		{
			manageRoled(rol, s);

			break;
		}
		case MessageType::COMMAND:
			break;
		}
	}
}

//PRIVADOS
void GameServer::manageLogin(Rol& rol, Socket *s)
{
	//Se añade el nuevo socket al vector de clientes
	std::cout << rol.getNick() << " ha entrado en el juego\n";
	
	//Mensaje de bienvenida
	GameMessage msg;
	msg.message = welcome;
	msg.type = MessageType::LOGIN;
			
	if(num_clientes == 0){
		//Cuando no hay clientes y entra el primero, el juego pasa a estado choosen
		state = GameState::CHOOSEN;

		client1.nick = rol.getNick();
		client1.socket = std::move(std::make_unique<Socket>(*s));
		socket.send(msg, *client1.socket);
	}
	else if(num_clientes == 1){
		client2.nick = rol.getNick();
		client2.socket = std::move(std::make_unique<Socket>(*s));
		socket.send(msg, *client2.socket);
	}
	else return;

		
	num_clientes++;
}

void GameServer::manageRoled(Rol& rol, Socket* s)
{
	
	//El mensaje siempre será de GameMessage
	std::cout << rol.getNick() << " ha escogido " <<
	      	(int)rol.getRol() << "\n";
			
	GameMessage msg;

	//Si aún no están todos los jugadores
	//Entonces aún no se pasa al estado de batalla
	if(num_clientes < MAX_CLIENTS){					
		msg.type = MessageType::ROLED;
		msg.message = "Ha escogido ";
	
		if(rol.getRol() == RolType::GUERRERO){
		msg.message += std::string("Guerrero\n");	
		}
		else if(rol.getRol() == RolType::MAGO){
		msg.message += std::string("Mago\n");	
		}
		else if(rol.getRol() == RolType::ASESINO){
		msg.message += std::string("Asesino\n");	
		}
	
		msg.message += std::string("\nEsperando rival...\n\0");

		if(rol.getNick() == client1.nick) socket.send(msg, *client1.socket);
		else if(rol.getNick() == client2.nick) socket.send(msg, *client2.socket);
				
		return;
	}
			
	//Si llega hasta aquí es porque ya se han alcanzado
	//el máximo número de clientes
	msg.message = "\nPasando a fase de batalla\n\0";
	msg.type = MessageType::INIT_BATTLE;
	
	socket.send(msg, *client1.socket);
	socket.send(msg, *client2.socket);
	
	state = GameState::MAINLOOP;
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
		Socket* socket = player->getSocket();

		std::string msg;	
		std::getline(std::cin, msg);
		
		switch(state){
		case GameState::CHOOSEN:
		{
			//Si el jugador ya ha escogido rol
			//Entonces será distinto a NONE, por tanto, 
			//no debería volver a escoger
			if(player->getRol() != RolType::NONE) 
			{
				std::cout << "\nYa se ha escogido Rol\n";
				break;
			}
			chooseRol(msg);

			break;
		}
		case GameState::MAINLOOP:
			state = GameState::WAITING;
			chooseAction(msg);
			break;
		case GameState::WAITING:
			std::cout << "\nEspere mientras su rival elige su movimiento\n";
			break;
		}
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
			std::cout << "\nEscriba un comando: \n";
			state = GameState::CHOOSEN;
			break;
		case MessageType::ROLED:
			std::cout << "\n" << m.message << "\n";
			break;
		case MessageType::INIT_BATTLE:
			std::cout << "\n" << m.message << "\n";
			std::cout << "\nEscriba un comando: \n";
			state = GameState::MAINLOOP;
			break;
		}
    	}
}

//PRIVADOS
void GameClient::chooseRol(std::string msg)
{
	Socket* socket = player->getSocket();
	if (msg == "M")
       	{
		player->setRolType(RolType::MAGO);
		Rol mago(player, MessageType::ROLED);
		
		int s = socket->send(mago, *socket);
		if(s == -1) std::cerr << "Error en el envío del mensaje ROL\n";
	}
	else if (msg == "G") 
	{
		player->setRolType(RolType::GUERRERO);
		Rol guer(player, MessageType::ROLED);
		
		int s = socket->send(guer, *socket);
		if(s == -1) std::cerr << "Error en el envío del mensaje ROL\n";
	}
	else if (msg == "A") 
	{
		player->setRolType(RolType::ASESINO);
		Rol asesino(player, MessageType::ROLED);
		
		int s = socket->send(asesino, *socket);
		if(s == -1) std::cerr << "Error en el envío del mensaje ROL\n";
	}
	else
	{
		std::cout << "No se reconoce el comando: " << msg <<
		       	"\nVuelva a intentarlo\n";
	}
}

void GameClient::chooseAction(std::string msg)
{
	Socket* socket = player->getSocket();
}
