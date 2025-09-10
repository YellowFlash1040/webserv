/*
while we are in a quote, and current character is not a closing quote,
just add characters to the value string until you found closing quote
or you hit the end of the string.
if we found a closing quote character - value is ready to become a token
if we didn't find an end of quote and there are no more characters - give an
error

if we are not in a quote

while current character is not a delimiter ('\n', ' ', '\t') or a special
character (';', '{' '}') or a quote character ("'", '"')
add characters to the value string

then, once value is ready to become a token,
check if there is a previous token,
and if there is and it's either an OPEN_BRACE or SEMICOLON,
it means that value has to be one of the directives, and I need to figure out
which one, if it's none of the directives - give an error and quit

otherwise if the DELIMITER is not an OPEN_BRACE or SEMICOLON, I need to add
previous token to the toke list with type VALUE. If it is a semicolom or one of
the braces, I still need to add the value to the tokens list as token with type
VALUE, but also I need to figure out the type of the delimiter and add it to the
tokens list with the right type

if I see an OPEN_BRACE, I need to check if the previous token has
value server, because if it is, it means that it's not a token
of type VALUE, but the token of type SERVER

token that is after open brace or after semicolon has to be one of the
directives

all of the tokens after a directive and before semicolon have to be values

if I see a '\n' or a ' ' or a '\t' it means that the token has ended,
and therefore if it's not empty I need to add it to the list of the tokens


if I found a directive and I haven't found a semicolon or an open brace yet, it
means that the token type has to be VALUE

*/
