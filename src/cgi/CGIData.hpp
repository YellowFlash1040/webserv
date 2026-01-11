#pragma once

#ifndef CGIDATA_HPP
# define CGIDATA_HPP

# include <utility>
# include <unistd.h>
# include <ctime>
# include <string>
# include "ResponseData.hpp"

struct CGIData
{
    pid_t pid = -1;
    int fd_stdin = -1;
    int fd_stdout = -1;
    time_t start_time;
    bool addedToEpoll = false;
    std::string input;
    std::string output;
    size_t input_sent = 0;
    ResponseData* response = nullptr;
};

#endif
