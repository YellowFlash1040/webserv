#include <iostream>
#include "FileReader.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Validator.hpp"

int main(void)
{
    const char* filepath = "./webserv.conf";
    try
    {
        std::string text = FileReader::readFile(filepath);
        auto tokens = Lexer::tokenize(text);
        auto directives = Parser::parse(tokens);
        Validator::validate(directives);
        std::cout << "Well done :)"
                  << "\n";
    }
    catch (const ConfigException& e)
    {
        std::cerr << "\033[1m";
        std::cerr << filepath << ":";
        std::cerr << "\033[0m";

        std::cerr << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
