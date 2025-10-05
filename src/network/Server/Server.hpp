#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <utility>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <errno.h>
# include <cstdlib>
# include <stdexcept>
# include <csignal>
# include <iostream>

# include "Config.hpp"
# include "MemoryUtils.hpp"
# include "NetworkEndpoint.hpp"

// HTTP connection manager
#include "../http/ConnectionManager/ConnectionManager.hpp"

#define GREEN "\033[0;32m"
#define RESET "\033[0m"

typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;
typedef struct epoll_event t_event;

extern volatile std::sig_atomic_t g_running;

class Server
{
		// Construction and destruction
	public:
		Server::Server(int port, Config config);
		~Server();
		void run(void);

	protected:
		// Properties
		// Methods

	private:
		// constants
		static constexpr int QUEUE_SIZE = 100;
		static constexpr int MAX_EVENTS = 50;
		// Properties
		int m_listeningSocket = -1;
		int m_epfd = -1; // event poll fd
		t_sockaddr_in m_address;
		ConnectionManager m_connMgr;
				
		// Methods
		void fillAddressInfo(int port);
		void setNonBlockingAndCloexec(int fd);
		void addSocketToEPoll(int socket, uint32_t events);
		void acceptNewClient(void);
		void processClient(int clientSocket);
};


#endif
