#pragma once

#ifndef DUPLICATE_SERVERID_EXCEPTION_HPP
# define DUPLICATE_SERVERID_EXCEPTION_HPP

# include <stdexcept>
# include <string>

# include "ValidatorException.hpp"

class DuplicateServerIdException : public ValidatorException
{
  public:
    DuplicateServerIdException(const std::string& serverName,
                               const std::string& listen);

    const char* what() const noexcept override;
};

#endif
