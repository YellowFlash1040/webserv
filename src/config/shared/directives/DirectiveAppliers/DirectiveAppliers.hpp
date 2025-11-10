#pragma once

#ifndef DIRECTIVE_APPLIERS_HPP
# define DIRECTIVE_APPLIERS_HPP

# include <map>
# include <vector>

namespace DirectiveAppliers
{

struct Replace
{
    template <typename U, typename V>
    void operator()(const U& value, V& target) const
    {
        target = value;
    }
};

struct AppendHead
{
    template <typename Value, typename Container>
    void operator()(const std::vector<Value>& value, Container& target) const
    {
        target.insert(target.begin(), value.begin(), value.end());
    }
};

struct AppendTail
{
    template <typename Value, typename Container>
    void operator()(const std::vector<Value>& value, Container& target) const
    {
        target.insert(target.end(), value.begin(), value.end());
    }
};

struct MergeMap
{
    template <typename T, typename U>
    void operator()(const std::map<T, U>& value, std::map<T, U>& target) const
    {
        for (const auto& it : value)
            target[it.first] = it.second;
    }
};

} // namespace DirectiveAppliers

#endif
