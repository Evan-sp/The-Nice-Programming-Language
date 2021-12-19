#include <iostream>
#include <fstream>
#include "lexer.hpp"

using namespace std;

int SourceFile::openFile(char filePath[]) {
	fileStream.open(filePath, fstream::in);
	if (!fileStream) {
		return 0;
	}
	return 1;
}

int SourceFile::peekNextChar() {
	return fileStream.peek();
}

int SourceFile::consumeNextChar() {
	return fileStream.get();
}

int Lexer::isOperator(int c){
	if(c == '%' || c == '/' || c == '*' || c == '+' 
	|| c == '-' || c == '=' || c == '<'){
		return 1;
	}
	return 0;
}

void Lexer::openSourceFile(char filePath[]) {
	sourceFile.openFile(filePath);
}

void Lexer::step(State& state, Token& token, char c) {
	switch (state) {
		case UNKNOWN_S: 
			if (isspace(c)) {
				sourceFile.consumeNextChar();
				return;
			} else if (isalpha(c)) { 
				token.type = IDENTIFIER_T;
				state = IDENTIFIER_S;
			} else if (isdigit(c)) {
				token.type = NUMBER_T; 
				state = NUMBER_S;
			} else if (c == '(') {
				token.type = OPEN_P_T;
				state = DONE_S;
			} else if (c == ')') {
				token.type = CLOSED_P_T;
				state = DONE_S;
			} else if (c == EOF) {
				token.type = EOF_T;
				state = DONE_S;
			} else if (isOperator(c)) {
				token.type = OPERATOR_T;
				state = DONE_S;
			} else {
				token.type = UNKNOWN_T;
				state = DONE_S;
			}
			token.value += c;
			sourceFile.consumeNextChar();
			break;
		case IDENTIFIER_S:
			if (isalpha(c) || isdigit(c)) { 
				token.value += c;
				sourceFile.consumeNextChar();
			} else {
				state = KEYWORD_S;
			}
			break;
		case NUMBER_S:
			if (isdigit(c) || c == '.') { 
				token.value += c;
				sourceFile.consumeNextChar();
			} else {
				state = DONE_S;
			}
			break;
		case KEYWORD_S:
			if (token.value == "return") {
				token.type = RETURN_T;
			} else if (token.value == "function") {
				token.type = DEFINITION_T;
			}
			state = DONE_S;
			break;
		case DONE_S:
			break;
	}
}

Token Lexer::getToken() {
	Token token(UNKNOWN_T, "");
	State state = UNKNOWN_S;
	while (state != DONE_S) {
		step(state, token, sourceFile.peekNextChar());
	}	
	return token;
}
