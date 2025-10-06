#include "Property.hpp"
#include <utility>

// Default constructor
template <typename T>
Property<T>::Property()
  : m_value()
  , m_isSet(false)
{
}

// Constructor from T
template <typename T>
Property<T>::Property(const T& value)
  : m_value(value)
  , m_isSet(true)
{
}

// Copy assignment operator (from T)
template <typename T>
Property<T>& Property<T>::operator=(const T& value)
{
    m_value = value;
    m_isSet = true;
    return *this;
}

// Move constructor (from T)
template <typename T>
Property<T>::Property(T&& value)
  : m_value(std::move(value))
  , m_isSet(true)
{
}

// Move assignment operator (from T)
template <typename T>
Property<T>& Property<T>::operator=(T&& value)
{
    m_value = std::move(value);
    m_isSet = true;
    return *this;
}

// Non-const getter
template <typename T>
T& Property<T>::value()
{
    return m_value;
}

// Const getter
template <typename T>
const T& Property<T>::value() const
{
    return m_value;
}

// Type conversion operator
//  So that instead of using:
//  T variable = p.value()
//  we could directly use
//  T variable = p
template <typename T>
Property<T>::operator T&()
{
    return m_value;
}

// Type conversion operator for const
template <typename T>
Property<T>::operator const T&() const
{
    return m_value;
}

// isSet getter
template <typename T>
bool Property<T>::isSet() const
{
    return m_isSet;
}

template <typename T>
bool& Property<T>::isSet()
{
    return m_isSet;
}

// Forward member access
template <typename T>
T* Property<T>::operator->()
{
    return &m_value;
}

template <typename T>
const T* Property<T>::operator->() const
{
    return &m_value;
}

template <typename T>
T& Property<T>::operator*()
{
    return m_value;
}

template <typename T>
const T& Property<T>::operator*() const
{
    return m_value;
}

template <typename T>
bool Property<T>::operator==(const Property<T>& other) const
{
    return m_value == other.m_value;
}

template <typename T>
bool Property<T>::operator!=(const Property<T>& other) const
{
    return m_value != other.m_value;
}

template <typename T>
bool Property<T>::operator==(const T& other) const
{
    return m_value == other;
}

template <typename T>
bool Property<T>::operator!=(const T& other) const
{
    return m_value != other;
}
