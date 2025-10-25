#pragma once

#ifndef CONFIGBLOCK_HPP
# define CONFIGBLOCK_HPP

# include "RequestContext.hpp"
# include "Property.hpp"
# include "DirectiveAppliers.hpp"

# include "EffectiveConfig.hpp"

class ConfigBlock
{
    // Construction and destruction
  public:
    virtual ~ConfigBlock() = default;

    // Methods
    virtual void applyTo(EffectiveConfig& ctx) const = 0;

  protected:
    // Methods
    template <typename T, typename U, typename ApplyProperty>
    void applyIfSet(const Property<T>& property, U& contextField,
                    ApplyProperty applyProperty) const
    {
        if (property.isSet())
            applyProperty(property.value(), contextField);
    }
};

#endif
