#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include "../ParsedRequest/ParsedRequest.hpp"
#include "../ServerResponse/ServerResponse.hpp"

class ClientState
{
	private:
		bool _headersDone;
		bool _headersSeparatedFromBody;
		bool _bodyDone;
		bool _readyToSend;
	
		std::string _rlAndHeadersBuffer; //accumulates incoming data until you detect the end of headers (\r\n\r\n)
		std::string _bodyBuffer;
		
		int _contentLength;
		bool _chunked;

		ParsedRequest _reqObj;
		ServerResponse _respObj;
		
	public:
		ClientState();
		~ClientState();
		ClientState(const ClientState& other);
		ClientState& operator=(const ClientState& other);
		ClientState(ClientState&& other) noexcept;
		ClientState& operator=(ClientState&& other) noexcept;

		// Getters
		const std::string& getRlAndHeadersBuffer() const;
		const std::string& getBodyBuffer() const;
		std::string getFullRequestBuffer() const;
		const ServerResponse& getRespObj() const;
		const ParsedRequest& getReqObj() const;
		size_t getContentLength() const;
		
		// Setters
		void setHeadersDone();
		void setBodyDone();
		void setHeadersSeparatedFromBody();
		void setContentLength(int value);
		void setChunked(bool value);
		void setRequest(const ParsedRequest& req);
		void setResponse(const ServerResponse& resp);
		void setReadyToSend(bool value);
		
		// Appenders
		void appendToRlAndHeaderBuffer(const std::string& data);
		void appendToBodyBuffer(const std::string& data);
		
		void clearRlAndHeaderBuffer();
		bool hasHeadersBeenSeparatedFromBody() const;
		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isChunked() const;
		bool isReadyToSend() const;
		void prepareForNextRequestBuffersOnly();
		void prepareForNextRequest();
		void appendToBuffers(const std::string& data);

};

#endif
