#include "Game.h"

//----------Clase Rol---------//
Rol::Rol()
{
	vida = 0;
	mana = 0;
	atk = 0;
	mana_r = 0;
}
Rol::~Rol(){}

void Rol::to_bin()
{
}

int Rol::from_bin(char* data)
{
}

//-------GET-SET----------//
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

void Rol::setRolType(RolType rt)
{
	rol_t = rt;
	switch(rol_t){
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

void Rol::setNick(std::string n)
{
	nick = n;
}

void Rol::setCommand(std::string c)
{
	command = c;
}

void Rol::setMsgType(MessageType newType)
{
	type = newType;
}

//----------Clase-GameMessage------------// 

void GameMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
}

int GameMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

    return 0;
}

//-----------Clase-GameServer----------//
GameServer::GameServer(const char* s, const char* p) : socket(s, p) {}

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

		switch(rol.getMsgType()){
		case MessageType::LOGIN:
			//Se añade el nuevo socket al vector de clientes
			clients.push_back(std::move(std::make_unique<Socket>(*s)));

			//Se le envía un mensaje de bienvenida
			//Se le pregunta por el Rol
			//Cuando responda, el cliente enviará un mensaje de tipo ROLED
			//para informa de que ha escogido el rol	
			break;
		case MessageType::LOGOUT:
			break;
		case MessageType::FINISH_ROUND:
			break;
		case MessageType::FINISH_GAME:
			break;
		case MessageType::ROLED:
			break;
		case MessageType::COMMAND:
			break;
		}
	}
}

//--------Clase-GameClient-------------//

GameClient::GameClient(const char* s, const char* p, const char* n) : socket(s, p), nick(n) 
{
}
void GameClient::command()
{
}

void GameClient::login()
{
	std::string c;
	//Asignacion del nickName
	rol.setNick(nick);
	//TTipo de mensaje
    	rol.setMsgType(MessageType::LOGIN);
	//Se le asigna un string vacio por si hubiera
	//algo escrito en el anterior
	rol.setCommand(c);
	//Se le avisa al server de la conexión
    	socket.send(rol, socket);
}

void GameClient::logout()
{
	//Funciona igual que el LOGIN
	std::string c;
	rol.setMsgType(MessageType::LOGOUT);
	rol.setCommand(c);
	socket.send(rol, socket);
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
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}

