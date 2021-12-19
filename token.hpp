#ifndef TOKEN_HPP
#define TOKEN_HPP
#include <string>
typedef enum {
	UNKNOWN_T,
	NUMBER_T,
	OPEN_P_T,
	CLOSED_P_T,
	EOF_T,
	OPERATOR_T,	
	IDENTIFIER_T,
	RETURN_T,
	ASSIGNMENT_T,
	DEFINITION_T,
} TokenType;

class Token {
public:
	TokenType type;
	std::string value;
	Token(TokenType type, std::string value) : type(type), value(value) {}
	Token() = default;
};
#endif
