#include <string.h>

#include "Serializable.h"
#include "Socket.h"
#include "Game.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase

	struct addrinfo hints;
	struct addrinfo *res;

	memset((void*) &hints, 0, sizeof(struct addrinfo));

	//IPv4 // UDP
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int r = getaddrinfo(address, port, &hints, &res);

	if(r != 0){
		std::cerr << "Error en [getaddrinfo]\n";
	}

	sd = socket(res->ai_family, res->ai_socktype, 0);

	if(sd == -1){
		std::cerr << "Error al crear el socket [socket]\n";
	}

	sa = *res->ai_addr;
	sa_len = res->ai_addrlen;

	freeaddrinfo(res);
}

int Socket::recv(Serializable &obj, Socket * &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);
    if ( bytes <= 0 )
    {
	std::cerr << "Error al recibir el número de bytes\n";
        return -1;
    }

    if ( sock == nullptr )
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    	//Serializar el objeto
    	obj.to_bin();

    	ssize_t bytes = sendto(sock.sd, obj.data(), obj.size(), 0, &sock.sa, sock.sa_len);

	if(bytes <= 0) 
		return -1;

	return 0;
}

bool operator== (const Socket &s1, const Socket &s2)
{
	std::cout << "COMPARATOR\n";
    	//Comparar los campos sin_family, sin_addr.s_addr y sin_port
    	//de la estructura sockaddr_in de los Sockets s1 y s2
    	//Retornar false si alguno difiere
	if(s1.sa.sa_family != s2.sa.sa_family) 
	{
		std::cout << "FAMILY !=\n";
		return false;
	}

	struct sockaddr_in* s1_aux = (struct sockaddr_in *) &(s1.sa);
	struct sockaddr_in* s2_aux = (struct sockaddr_in *) &(s2.sa); 
	std::cout << "ADDR == \n";	
	if (s1_aux->sin_addr.s_addr != s2_aux->sin_addr.s_addr)
	{
		std::cout << "ADDR !=\n";
		return false;
	}
	if(s1_aux->sin_port != s2_aux->sin_port)
	{ 
		std::cout << "PORT !=\n";
		return false;
	}
	return true;
};


std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

