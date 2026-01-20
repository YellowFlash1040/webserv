#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <queue>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../Request/RequestData/RequestData.hpp"
#include "../../HttpMethod/HttpMethod.hpp"
#include "../../Response/RawResponse/RawResponse.hpp"
#include "../../utils/debug.hpp"
#include "../../cgi/CGIManager.hpp"

class ClientState
{
	private:
		// Properties
		
		// Raw requests from the client (byte level)
		std::deque<RawRequest> m_rawRequests;

		// Responses ready to be sent on the socket
		std::queue<ResponseData> m_respDataQueue;

		std::vector<CGIData> m_activeCGIs;

	public:
		// Construction and destruction
		ClientState();
		~ClientState();
		ClientState(const ClientState& other) = default;
		ClientState& operator=(const ClientState& other) = default;
		ClientState(ClientState&& other) noexcept = default;
		ClientState& operator=(ClientState&& other) noexcept = default;

		// Accessors
		RawRequest& getLatestRawReq();
		bool hasPendingResponseData() const;
		bool hasCompleteRawRequest() const;
		ResponseData& frontResponseData();
		const std::queue<ResponseData>& getResponseQueue() const;
		
		// Methods
		RawRequest& addRawRequest();
		void enqueueResponseData(const ResponseData& resp);
		RawRequest popFrontRawRequest();
		void popFrontResponseData();

		ResponseData& backResponseData();

		CGIData& createActiveCgi(RequestData& req, Client& client,
														 const std::string& interpreter,
														 const std::string& scriptPath, ResponseData* resp);

		std::vector<CGIData>& getActiveCGIs();
		CGIData* findCgiByPid(pid_t pid);
		void removeCgi(pid_t pid);
		void clearActiveCGIs();
};

#endif
