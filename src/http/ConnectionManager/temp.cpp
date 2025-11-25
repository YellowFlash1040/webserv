// bool parseRawRequest(RawRequest& rawReq)
// {
//     // Parse headers if not done
//     if (!rawReq.isHeadersDone())
//     {
//         rawReq.separateHeadersFromBody();
//         if (rawReq.isBadRequest())
//         {
//             DBG("[parseRawRequest] Bad request detected");
//             rawReq.setRequestDone();
//             return true;
//         }

//         if (!rawReq.isHeadersDone())
//         {
//             DBG("[parseRawRequest]: headers are not finished yet");
//             return false;
//         }
//     }

//     // Parse body if needed
//     if (!rawReq.isBadRequest() && rawReq.isHeadersDone() && !rawReq.isBodyDone())
//     {
//         rawReq.appendBodyBytes(rawReq.getTempBuffer());
//         if (!rawReq.isBodyDone())
//         {
//             DBG("[parseRawRequest]: body not finished yet");
//             return false;
//         }
//     }

//     // Full request done
//     if ((rawReq.isHeadersDone() && rawReq.isBodyDone()) || rawReq.isBadRequest())
//     {
//         DBG("[parseRawRequest]: request done");
//         rawReq.setRequestDone();
//         return true;
//     }

//     return false;
// }

// //processReqs becomes cleaner
// size_t ConnectionManager::processReqs(int clientId, const std::string& data)
// {
//     DBG("DEBUG: processReqs: ");
//     auto it = m_clients.find(clientId);
//     if (it == m_clients.end())
//         return 0;

//     ClientState& clientState = it->second;
//     RawRequest& rawReq = clientState.getLatestRawReq();

//     // Append all incoming bytes to the temp buffer
//     rawReq.appendTempBuffer(data);
//     DBG("[processReqs] tempBuffer is |" << rawReq.getTempBuffer() << "|");

//     size_t parsedCount = 0;

//     while (true)
//     {
//         RawRequest& rawReq = clientState.getLatestRawReq();
//         bool done = parseRawRequest(rawReq);

//         if (!done)
//             break;

//         parsedCount++;

//         std::string leftovers = rawReq.getTempBuffer();
//         if (!leftovers.empty())
//         {
//             DBG("[processReqs]: leftovers exist, adding new RawRequest");
//             RawRequest& newReq = clientState.addRawRequest();
//             newReq.setTempBuffer(leftovers);
//             continue;
//         }
//         else
//         {
//             break;
//         }
//     }

//     return parsedCount;
// }


