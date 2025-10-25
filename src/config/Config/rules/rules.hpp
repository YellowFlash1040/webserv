#pragma once

#ifndef RULES_HPP
# define RULES_HPP

namespace Rules
{

void applyErrorPages(const std::vector<ErrorPage>& errorPages,
                     std::map<HttpStatusCode, std::string>& target);

template <typename U, typename V>
void replace(const U& value, V& target);

template <typename Value, typename Container>
void appendHead(const std::vector<Value>& value, Container& target);

# include "rules.tpp"

} // namespace Rules

#endif