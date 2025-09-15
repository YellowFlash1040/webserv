#include <iostream>
#include "Lexer.hpp"
#include "Parser.hpp"

int main(void)
{
    try
    {
        auto tokens = Lexer::tokenize("location\n/images {\nroot /var/www;\n}");
        auto directives = Parser::parse(tokens);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
