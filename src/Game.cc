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
std::string Rol::getCommand() const
{
	return command;
}

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

void Rol::addVida(int v)
{
	vida += v;
}

void Rol::addMana() 
{
	mana += mana_r;
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

	commands = "\nLista de acciones -> Acción:Maná\n" +
		std::string("Ataque básico -> B (no consume maná)\n") +
		std::string("Ataque fuerte -> F:N (Consume N de maná)\n") +
		std::string("Mitigar daño  -> M\n") + 
		std::string("Salir         -> S\n");

	//Iniciamos los clientes como no usados
	clients[0].used = false;
	clients[1].used = false;
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
		 * procede al gamestate de BATTLE.
		 */
		Rol rol;
		Socket* s = nullptr;
		
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
			manageLogout(rol, s);
			break;
		case MessageType::FINISH_ROUND:
			break;
		case MessageType::FINISH_GAME:
			break;
		case MessageType::ROLED:
			manageRoled(rol, s);
			break;
		case MessageType::COMMAND:
			manageCommand(rol, s);
			break;
		}
	}
}

//PRIVADOS
void GameServer::manageLogin(Rol& rol, Socket *s)
{
	//Se añade el nuevo socket al vector de clientes
	std::cout << "\n" << rol.getNick() << " ha entrado en el juego\n";
	
	//Mensaje de bienvenida
	GameMessage msg;
	msg.message = welcome;
	msg.type = MessageType::LOGIN;
			
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!clients[i].used){
			clients[i].used = true;
			clients[i].stats = rol;
			clients[i].socket = std::move(std::move(std::make_unique<Socket>(*s)));
			socket.send(msg, *clients[i].socket);

			break;
		}
	}

	num_clientes++;
}

void GameServer::manageRoled(Rol& rol, Socket* s)
{
	//El mensaje siempre será de GameMessage
	std::cout << rol.getNick() << " ha escogido " <<
	      	(int)rol.getRol() << "\n";
	
	GameMessage msg;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		//Cuando el jugador escoge rol, se guardan sus datos en el server
		if(rol.getNick() == clients[i].stats.getNick()){
			clients[i].stats = rol;
			msg.message = "Ha escogido: ";
			msg.type = MessageType::ROLED;
			if(clients[i].stats.getRol() == RolType::GUERRERO){
				msg.message += std::string("Guerrero\n");
			}
			else if(clients[i].stats.getRol() == RolType::MAGO){
				msg.message += std::string("Mago\n");
			}
			else if(clients[i].stats.getRol() == RolType::ASESINO){
				msg.message += std::string("Asesino\n");
			}

			roledPlayers++;
			//Envío del rol escogido al jugador
			socket.send(msg, *s);
			break;
		}		
	}	
	
	//Si aún no han llegado otros jugadores
	if(roledPlayers < MAX_CLIENTS){
		msg.message = std::string("\nEsperando rival...\n");
		socket.send(msg, *s);
	       	return;
	}
			
	//Si llega hasta aquí es porque los clientes han elegido roles
	msg.type = MessageType::INIT_BATTLE;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(clients[i].socket == nullptr){
			std::cout << "ES NULL\n";
			continue;
		}

		msg.message = "\nPasando a fase de batalla\n";
		msg.message += "Informe enemigo: ";
		if (i == 0){
			msg.message += informeEnemigo(1);
		}
		else{
			msg.message += informeEnemigo(0);
		}
		
		msg.message += informe(i);	
		
		//Lista de comandos
		msg.message += commands;
		socket.send(msg, *clients[i].socket);
	}	
}

void GameServer::manageLogout(Rol& rol, Socket* s)
{
	int i = 0;
	for(; i < MAX_CLIENTS; i++){
		if(clients[i].stats.getNick() == rol.getNick())
		{
			num_clientes--;
			clients[i].used = false;
			std::cout << "CLIENT FALSE\n";

			//Mensaje de despedida
			GameMessage msg;
			msg.type = MessageType::LOGOUT;
			msg.message = "Hasta la vida\n";
			socket.send(msg, *clients[i].socket);
			
			std::cout << "ENVIO DE LOGOUT\n";
			//Liberación del socket
			clients[i].socket = nullptr;
			std::cout << "LIBERACION DE SOCKET\n";
			
			break;
		}
	}

	if(i == MAX_CLIENTS){
		std::cout << "El jugador ya se había desconectado previamente\n";
	}
}

void GameServer::manageCommand(Rol& rol, Socket* s)
{
	int i = 0;

	GameMessage msg;
	for(;i < MAX_CLIENTS; i++)
	{
		//Le asignamos el comando al cliente
		//para procesarlo después en caso de que los dos
		//hayan escogido comando
		if(rol.getNick() == clients[i].stats.getNick())
		{
			clients[i].stats.setCommand(rol.getCommand());
			break;
		}
	}

	//Comprobación de que si es ataque fuerte se disponga del maná necesario
	if(clients[i].stats.getCommand()[0] == 'F' &&
	   (int)(clients[i].stats.getCommand()[2] - '0') > clients[i].stats.getMana())
	{
		std::cout << "MANA ENTRANTE: " << 
			clients[i].stats.getCommand()[2] - '0' << std::endl;;
		msg.message = "\nNo hay maná suficiente para efectuar el ataque\n";
		msg.message += std::string("Su maná: ") +
		       	std::to_string(clients[i].stats.getMana());
		msg.message += std::string("\n");
		msg.type = MessageType::INIT_BATTLE;
		socket.send(msg, *s);
		return;
	}

	num_commands++;
	//En caso de que los dos jugadores no hayan 
	//escogido un comando, entonces se le dice al que
	//haya escogido que espere
	if(num_commands < MAX_CLIENTS) {
		msg.message = "\nEsperando rival...\n";
		msg.type = MessageType::COMMAND;
		socket.send(msg, *s);
		enemigo = i;
		return;
	}

	//En este caso el orden del procesamiento de los mensajes nos da igual, dado
	//que se puede empatar. En caso de que los jugadores bajen por debajo de 0 
	//se considerará empate.
	
	//Procesando al último que envío el mensaje
	//Si mitiga daño entonces no hace falta entrar aquí
	if(rol.getCommand() != "M"){
		if(rol.getCommand() == "B")
		{
			int pupa = clients[i].stats.getAtk();
			if(clients[enemigo].stats.getCommand() == "M")
			{
				pupa -= (int)(pupa * 0.1);
			}
			clients[enemigo].stats.addVida(-pupa);
		}
		else if(rol.getCommand()[0] == 'F')
		{
			//Procesamos comando enemigo
			if(rol.getCommand() == "B")
			{
			}
		}
		else
		{	

		}
	}

	//Procesando al otro jugador
	if(clients[enemigo].stats.getCommand() != "M"){
		
	}

	//Comprobación de vidas
	//Empate
	if(clients[i].stats.getVida() <= 0 &&
	   clients[enemigo].stats.getVida() <= 0){
		msg.message = "\nHas empatado\n";
		msg.type = MessageType::FINISH_GAME;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			socket.send(msg, *clients[i].socket);
		}

		return;
	}
	//Ha ganado el otro
	else if(clients[i].stats.getVida() <= 0)
	{
		msg.message = "\nHas ganado\n";
		msg.type = MessageType::FINISH_GAME;
		socket.send(msg, *clients[enemigo].socket);

		msg.message = "\nHas perdido\n";
		socket.send(msg, *clients[i].socket);

		return;
	}
	//Has ganadp
	else if(clients[enemigo].stats.getVida() <= 0)
	{
		msg.message = "\nHas ganado\n";
		msg.type = MessageType::FINISH_GAME;
		socket.send(msg, *clients[i].socket);

		msg.message = "\nHas perdido\n";
		socket.send(msg, *clients[enemigo].socket);

		return;
	}
}

std::string GameServer::informeEnemigo(const int& i)
{
	//Rol
	std::string msg;
	msg += std::string("Rol: ");
	if(clients[i].stats.getRol() == RolType::GUERRERO){
		msg += std::string("Guerrero ");
	}
	else if(clients[i].stats.getRol() == RolType::ASESINO){
		msg += std::string("Asesino ");
	}
	else if(clients[i].stats.getRol() == RolType::MAGO){
		msg += std::string("Mago ");
	}
	//Vida
	msg += std::string("Vida: ") + 
		std::to_string(clients[i].stats.getVida());
	//Maná
	msg += std::string(" Maná: ") + 
		std::to_string(clients[i].stats.getMana());
	//Ataque
	msg += std::string(" Ataque: ") + 
		std::to_string(clients[i].stats.getVida());

	return msg;
}

std::string GameServer::informe(const int& i)
{
	std::string msg;

	//Vida
	msg += std::string("\nSus atributos: Vida: " +
		       std::to_string(clients[i].stats.getVida()));
	//Mana
	msg += std::string("/ Maná: ") +
		       std::to_string(clients[i].stats.getMana());
	//Ataque
	msg += std::string("/ Ataque: ") +
	       		std::to_string(clients[i].stats.getAtk());
	msg += std::string("\n");

	return msg;
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
	Rol rol = Rol(this, MessageType::LOGOUT);
	int send = socket.send(rol, socket);
	if(send == -1){
		std::cerr << "Error en LOGOUT\n";
	}

	std::cout << "LOGOUT ENVIADO\n";
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

GameClient::GameClient(const char* s, const char* p, const char* n) : 
	exit(false),
	exit_i(false)
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
	while (!exit_i)
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
		case GameState::BATTLE:
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
    	while(!exit_i)
    	{
		GameMessage m;
		player->getSocket()->recv(m);
		/*
		 * Por parte del servidor siempre se reciben mensajes GameMessage.
		 * Cuando estos llegan se muestran por pantalla siempre.
		 * Tipos de mensaje
		 * 
		 * LOGIN: Mostrará el mensaje de Bienvenida y pasará al estado
		 * 
		 * CHOOSEN para escoger el rol.
		 * 
		 * INIT_BATTLE: Mostrará el mensaje de que se ha encontrado rival 
		 * junto a la información del estado de la batalla y se pasará al
		 * estado de batalla BATTLE.
		 *
		 * COMMAND: Si recibe un mensaje de este tipo siginifica que el otro
		 * jugador está escogiendo un comando, por tanto se pasa al estado
		 * WAITING
		 *
		 * FINISH ROUND: Mostrará el informe de la ronda y la información
		 * del estado del jugador . Se pasa al estado BATTLE.
		 *
		 * FINISH_GAME:  
		 */	

		//Mensaje del servidor
		std::cout << "\n" << m.message << "\n";

		switch(m.type){	
		case MessageType::LOGIN:
			std::cout << "\nEscriba un comando: \n";
			state = GameState::CHOOSEN;
			break;
		case MessageType::ROLED:
			break;
		case MessageType::INIT_BATTLE:
			std::cout << "\nEscriba un comando: \n";
			state = GameState::BATTLE;
			break;
		case MessageType::COMMAND:
			state = GameState::WAITING;
			break;
		case MessageType::FINISH_ROUND:
			state = GameState::BATTLE;
			break;
		case MessageType::FINISH_GAME:
			state = GameState::FINISH;
			break;
		case MessageType::LOGOUT:
			std::cout << "EXIT TRUE\n";
			exit = true;
			Socket * delS = player->getSocket();
			delS->finish();

			delete player;
			break;
		}
    	}
}

//PRIVADOS
void GameClient::chooseRol(const std::string& msg)
{
	Socket* socket = player->getSocket();
	if(msg == "M")
       	{
		player->setRolType(RolType::MAGO);
		Rol mago(player, MessageType::ROLED);
		
		int s = socket->send(mago, *socket);
		if(s == -1) std::cerr << "Error en el envío del mensaje ROL\n";
	}
	else if(msg == "G") 
	{
		player->setRolType(RolType::GUERRERO);
		Rol guer(player, MessageType::ROLED);
		
		int s = socket->send(guer, *socket);
		if(s == -1) std::cerr << "Error en el envío del mensaje ROL\n";
	}
	else if(msg == "A") 
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

void GameClient::chooseAction(const std::string& msg)
{
	Socket* socket = player->getSocket();
	//En caso de escribir cualquiera de los 3 comandos de acción permitidos
	//se procesan y se envían al servidor como MessageType::COMMAND
	// Atk básico                 Atk fuerte               Rec Maná
	if(msg == "B" || (msg[0] == 'F' && msg.size() <= 3) || msg == "M") 
	{
		std::cout << "Comando: " << msg << "\n";
		Rol rol(player, MessageType::COMMAND);
		rol.setCommand(msg);
		int s = socket->send(rol, *socket);
		if(s == -1){
			std::cerr << "Error en el envío del COMANDO\n";
		}
	}	
	else if(msg == "S")	//Salir
	{
		exit_i = true;
		player->logout();
	}
	else
	{
		std::cout << "No se reconoce el comando\n";
	}
}
