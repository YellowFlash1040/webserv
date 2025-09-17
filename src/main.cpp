#include <iostream>
#include "FileReader.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main(void)
{
    try
    {
        std::string text = FileReader::readFile("./webserv.conf");
        auto tokens = Lexer::tokenize(text);
        auto directives = Parser::parse(tokens);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
