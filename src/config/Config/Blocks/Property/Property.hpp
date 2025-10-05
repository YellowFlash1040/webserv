#pragma once

#ifndef PROPERTY_HPP
# define PROPERTY_HPP

# include <utility>

template <typename T>
class Property
{
    // Construction and destruction
  public:
    Property();
    explicit Property(const T& value);
    explicit Property(T&& value);
    // Assigment operators for T
    Property<T>& operator=(const T& value);
    Property<T>& operator=(T&& other);
    // Copy/move constructors and assignment
    Property(const Property&) = default;
    Property(Property&&) noexcept = default;
    Property& operator=(const Property&) = default;
    Property& operator=(Property&&) noexcept = default;
    ~Property() = default;

    // Class specific features
  public:
    // Implicit conversion to T
    operator T&();
    operator const T&() const;
    // Forward member access
    T* operator->();
    const T* operator->() const;

    T& operator*();
    const T& operator*() const;
    // Explicit getters
    T& value();
    const T& value() const;
    bool& isSet();
    bool isSet() const;

    // Add iterator support
    auto begin() { return m_value.begin(); }
    auto end() { return m_value.end(); }
    auto begin() const { return m_value.begin(); }
    auto end() const { return m_value.end(); }

  private:
    // Properties
    T m_value;
    bool m_isSet;
};

# include "Property.tpp"

#endif
