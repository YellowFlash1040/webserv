#pragma once

#ifndef HTTPMETHOD_HPP
# define HTTPMETHOD_HPP

# include <string>
# include <stdexcept>
# include <unordered_map>
# include <vector>

enum class HttpMethod
{
    NONE,
    GET,
    POST,
    DELETE
};

// class HttpMethod
// {
//   public:
//     // Constructors
//     HttpMethod();
//     explicit HttpMethod(const std::string& value);
//     HttpMethod(const HttpMethod& other) = default;
//     HttpMethod& operator=(const HttpMethod& other) = default;
//     HttpMethod(HttpMethod&& other) noexcept = default;
//     HttpMethod& operator=(HttpMethod&& other) noexcept = default;
//     ~HttpMethod() = default;

//     // Accessors
//     std::string toString() const;
//     HttpMethodEnum value() const;

//     // Methods
//     static bool isValid(const std::string& value);
//     static void setDefaultHttpMethods(std::vector<HttpMethod>& httpMethods);
    
    
//   private:
//     HttpMethodEnum m_value;

//     // Internal helpers
//     static HttpMethodEnum stringToEnum(const std::string& value);
//     static std::string enumToString(HttpMethodEnum method);
// };

#endif
