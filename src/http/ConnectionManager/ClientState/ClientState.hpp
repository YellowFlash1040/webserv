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

/**
 * @brief ClientState holds the state of a single connected client.
 *
 * Container choices:
 *
 * - Requests (`m_requests`): std::deque<RawRequest>
 *   - Reason: We need efficient access to both the front and back of the container.
 *     - `back()` is used to append incoming bytes to the current (possibly partial) request.
 *     - `front()` is used to process the oldest request first when generating responses.
 *     - New requests (e.g., leftovers from parsing) are added at the back.
 *   - We could not use `std::queue` here because it does not provide `back()` access,
 *     which is required to append to the current incomplete request.
 *   - No random access is required. The deque naturally supports both partial requests
 *     and multiple pipelined requests in FIFO order.
 *
 * - Responses (`m_responses`): std::queue<ResponseData>
 *   - Reason: We only need strict FIFO behavior.
 *     - Push new responses to the back as they are generated.
 *     - Pop responses from the front when sending to the client.
 *   - Random access and iteration are not needed, so `queue` expresses intent clearly
 *     and provides a safe, minimal interface.
 */
class ClientState
{
	private:
		// Properties
		std::deque<RawRequest> m_requests;
		std::queue<ResponseData> m_responses;
		std::vector<CGIData> m_activeCGIs;

	public:
		// Construction and destruction
		ClientState();
		ClientState(const ClientState& other) = default;
		ClientState& operator=(const ClientState& other) = default;
		ClientState(ClientState&& other) noexcept = default;
		ClientState& operator=(ClientState&& other) noexcept = default;
		~ClientState();

		// Accessors
		bool hasCompleteRequest() const;
		bool hasPendingResponse() const;
		RawRequest& backRequest();
		ResponseData& backResponse(); //the connection header is changed by CGI
		const ResponseData& frontResponse() const;
		const std::queue<ResponseData>& responses() const;
		std::vector<CGIData>& activeCGIs();
		
		// Methods
		RawRequest& addRequest();
		void enqueueResponse(const ResponseData& resp);
		RawRequest popFrontRequest();
		void popFrontResponse();
		CGIData& createActiveCgi(RequestData& req, Client& client,
														 const std::string& interpreter,
														 const std::string& scriptPath, ResponseData* resp);
		CGIData* findCgiByPid(pid_t pid);
		void removeCgi(pid_t pid);
		void clearActiveCGIs();
		std::vector<CGIData*> getTimedOutCGIs(time_t now, time_t timeout);
};

#endif
