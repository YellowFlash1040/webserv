#pragma once

#ifndef DIRECTIVEWRONGPARENTEXCEPTION_HPP
# define DIRECTIVEWRONGPARENTEXCEPTION_HPP

# include <stdexcept>
# include <string>

class DirectiveWrongParentException : public std::exception
{
  public:
    DirectiveWrongParentException(const std::string& name,
                                  const std::string& requiredParent);

    const char* what() const noexcept override;

  private:
    std::string m_message;
};

#endif
