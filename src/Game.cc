#include "Game.h"

Rol::Rol(uint8_t t){
	type = t;
	/*
 	* Inicialización de las variables del rol
 	*/
	switch(type){
	case MAGO:
		vida = M_VIDA;
		mana = M_MANA;
		atk = M_ATK;
		mana_r = M_MR;
		break;
	case GUERRERO:
		vida = G_VIDA;
		mana = G_MANA;
		atk = G_ATK;
		mana_r = G_MR;
		break;
	case ASESINO:
		vida = G_VIDA;
		mana = G_MANA;
		atk = G_ATK;
		mana_r = G_MR;
		break;
	case default:
		
		break;
	}
}





// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameServer::do_messages()
{
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
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameClient::login()
{
    std::string msg;

    GameMessage em(nick, msg);
    em.type = GameMessage::LOGIN;

    socket.send(em, socket);
}

void GameClient::logout()
{
    // Completar
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

