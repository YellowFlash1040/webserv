#include "Lexer.hpp"

int main(void)
{

    auto tokens = Lexer::tokenize("location\n/images {\nroot /var/www;\n}");

    return 0;
}
