#ifndef REQUEST_CONTEXT_PRINTER_HPP
#define REQUEST_CONTEXT_PRINTER_HPP

#include "../config/RequestContext/RequestContext.hpp"

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
#define TEAL "\033[36m"
#define BLUE "\033[38;2;100;149;237m"
#define RED "\033[31m"
#define MINT "\033[38;2;150;255;200m"
#define RESET "\033[0m"


void printReqContext(const RequestContext& ctx);

#endif