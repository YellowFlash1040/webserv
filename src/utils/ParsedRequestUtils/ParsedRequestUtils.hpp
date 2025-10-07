#ifndef PARSEDREQUESTUTILS_HPP
#define PARSEDREQUESTUTILS_HPP

#include "ClientState.hpp"
#include "ParsedRequest.hpp"
#include <iostream>

void printRequest(ClientState& clientState, size_t i);
void printAllRequests(ClientState& clientState);
void printSingleRequest(const ParsedRequest& req, size_t i);
void printBodyBuffers(ParsedRequest& req);

#endif