#pragma once

#ifndef DIRECTIVENOTALLOWEDEXCEPTION_HPP
# define DIRECTIVENOTALLOWEDEXCEPTION_HPP

# include <stdexcept>
# include <string>

class DirectiveNotAllowedException : public std::exception
{
  public:
    DirectiveNotAllowedException(const std::string& name,
                                 const std::string& context);

    const char* what() const noexcept override;

  private:
    std::string m_message;
};

#endif
