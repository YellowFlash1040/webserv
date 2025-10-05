#pragma once

#ifndef CONFIGBLOCK_HPP
# define CONFIGBLOCK_HPP

# include "RequestContext.hpp"

class ConfigBlock
{
    // Construction and destruction
  public:
    virtual ~ConfigBlock() = default;

    // Methods
    virtual void applyTo(RequestContext& ctx) const = 0;

  protected:
    // Methods
    template <typename T, typename U>
    void applyIfSet(const T& property, U& contextField) const
    {
        if (property.isSet())
            contextField = property;
    }
};

#endif
