#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

class ClientState
{
	public:
		// Constructors / Destructor
		ClientState();
		~ClientState();

		// Copy constructor / assignment
		ClientState(const ClientState& other);
		ClientState& operator=(const ClientState& other);

		// Move constructor / assignment
		ClientState(ClientState&& other) noexcept;
		ClientState& operator=(ClientState&& other) noexcept;

		// Getters
		const std::string& getHeaderBuffer() const;
		const std::string& getBodyBuffer() const;
		std::string getFullRequestBuffer() const;
		const std::string& getRawHeaderBuffer() const;
		const std::string& getRawBodyBuffer() const;
		const Response& getRespObj() const;
		const Request& getRequest() const;
		
		bool isHeadersComplete() const;
		size_t getContentLength() const;
		bool isChunked() const;
		bool isReadyToSend() const;
		void prepareForNextRequest();

		// Appenders
		void appendToHeaderBuffer(const std::string& data);
		void appendToBodyBuffer(const std::string& data);
		
		// Setters
		void setHeadersComplete(bool value);
		void setContentLength(int value);
		void setChunked(bool value);
		void setRequest(const Request& req);
		void setResponse(const Response& resp);
		void setReadyToSend(bool value);
		
	private:
		std::string _headerBuffer;
		std::string _bodyBuffer;
		bool _headersComplete;
		int _contentLength;
		bool _chunked;
		Request _request;
		Response _respObj;
		bool _readyToSend;
};

#endif
