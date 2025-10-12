#include "Server.hpp"

// --------------CONSTRUCTION AND DESTRUCTION--------------

// Default constructor
Server::Server(int port, Config& config)
	: m_connMgr(config)
{
	fillAddressInfo(port);

	m_listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listeningSocket == -1)
		throw std::runtime_error("socket");
	setNonBlockingAndCloexec(m_listeningSocket);

	int opt = 1;
	if (setsockopt(m_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
				   sizeof(opt))
		== -1)
		throw std::runtime_error("setsockopt");
	if (bind(m_listeningSocket, (t_sockaddr*)(&m_address), sizeof(m_address))
		== -1)
		throw std::runtime_error("bind");
	if (listen(m_listeningSocket, QUEUE_SIZE) == -1)
		throw std::runtime_error("listen");
}

// Destructor
Server::~Server()
{
	if (m_epfd != -1)
		close(m_epfd);
	if (m_listeningSocket != -1)
		close(m_listeningSocket);
}

// ---------------------------ACCESSORS-----------------------------

// ---------------------------METHODS-----------------------------

void Server::fillAddressInfo(int port)
{
	ft::bzero(&m_address, sizeof(m_address));

	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(port);
	m_address.sin_addr.s_addr = htonl(INADDR_ANY);
}

void Server::setNonBlockingAndCloexec(int fd)
{
	// Non-blocking
	int status_flags = fcntl(fd, F_GETFL, 0);
	if (status_flags == -1)
		throw std::runtime_error("fcntl getfl");

	int result = fcntl(fd, F_SETFL, status_flags | O_NONBLOCK);
	if (result == -1)
		throw std::runtime_error("fcntl setfl");

	// Close-on-exec
	int fd_flags = fcntl(fd, F_GETFD, 0);
	if (fd_flags == -1)
		throw std::runtime_error("fcntl getfd");

	result = fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC);
	if (result == -1)
		throw std::runtime_error("fcntl setfd");
}

void Server::run(void)
{
	m_epfd = epoll_create(1);
	if (m_epfd == -1)
		throw std::runtime_error("epoll_create");

	addSocketToEPoll(m_listeningSocket, EPOLLIN);

	t_event FDs[MAX_EVENTS];
	while (g_running)
	{
		int readyFDs = epoll_wait(m_epfd, FDs, MAX_EVENTS, -1);
		printf ("readyFDs is %d\n", readyFDs);
		if (readyFDs == -1)
		{
			if (errno == EINTR)
				continue; // interrupted by signal, retry
			throw std::runtime_error("epoll_wait");
		}
		for (int i = 0; i < readyFDs; ++i)
		{
			if (FDs[i].data.fd == m_listeningSocket)
			{
				acceptNewClient();
			}
			else
				processClient(FDs[i].data.fd);
		}
	}
}

void Server::addSocketToEPoll(int socket, uint32_t events)
{
	t_event e;
	e.data.fd = socket;
	e.events = events;

	int result = epoll_ctl(m_epfd, EPOLL_CTL_ADD, socket, &e);
	if (result == -1)
		throw std::runtime_error("epoll_ctl");
}

void Server::acceptNewClient()
{
	int clientId = accept(m_listeningSocket, NULL, NULL);
	if (clientId == -1)
		throw std::runtime_error("accept");

	setNonBlockingAndCloexec(clientId);
	addSocketToEPoll(clientId, EPOLLIN);
	m_connMgr.addClient(clientId);
	
}

void Server::processClient(int clientId)
{
	char buf[8192];
	int n = read(clientId, buf, sizeof(buf) - 1);

	if (n > 0)
	{
		buf[n] = '\0';
		std::cout << GREEN << "\nDEBUG[SERVER]:" << RESET << " read " << n << " bytes: \n" << buf << "\n";
		// Pass incoming data to ConnectionManager
		std::string data(buf, n);
		
		bool anyRequestDone = m_connMgr.processData(clientId, data);

		std::cout << GREEN << "DEBUG[SERVER]:" << RESET << " any request processed? " << anyRequestDone << "\n";
		
		//Send all ready responses
		std::string respStr;
		
	if (anyRequestDone)
	{
		while (m_connMgr.hasPendingResponses(clientId))
		{
		   std::string respStr = m_connMgr.getNextResponseString(clientId);
		   write(clientId, respStr.c_str(), respStr.size());
		}
	}

		// Only remove client if last request indicates Connection: close
		if (m_connMgr.clientSentClose(clientId))
		{
			printf ("CLOSE\n");
			m_connMgr.removeClient(clientId);
			close(clientId);
		}
	}
		
	else if (n == 0)
	{
		std::cout << GREEN << "DEBUG:" << RESET << " client closed connection, fd=" << clientId << "\n";
		std::cout << "Client closed the socket on their end, fd=" << clientId << "\n";
		m_connMgr.removeClient(clientId);
		close(clientId);
	}
		
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			std::cout << GREEN << "DEBUG:" << RESET << " no data available yet (EAGAIN)\n";
			return;
		}
		perror("read");
		throw std::runtime_error("read failed");
	}
}

