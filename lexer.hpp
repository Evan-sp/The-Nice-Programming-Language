#include <fstream>
#include "token.hpp"
typedef enum {
	UNKNOWN_S,
	IDENTIFIER_S,
	NUMBER_S,
	KEYWORD_S,
	DONE_S
} State;

class SourceFile {
public:
	std::fstream fileStream;
	int openFile(char filePath[]);
	int peekNextChar();
	int consumeNextChar();
};

class Lexer {
private:
	int isOperator(int c);
	std::pair<TokenType, std::string> tokenize(char c);
	void step(State& state, Token& token, char c);
public:
	SourceFile sourceFile;
	void openSourceFile(char filePath[]);
	Token getToken();
};
