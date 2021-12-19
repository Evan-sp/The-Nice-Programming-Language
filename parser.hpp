#include "lexer.hpp"
class Parser {
private:
	int accept(TokenType tokenType);
	int expect(TokenType tokenType);
	Lexer lexer; 
	Token token;
public:
	void nextToken();
	void openSourceFile(char filePath[]);
	std::unique_ptr<FunctionASTNode> parseProgram();
	std::unique_ptr<FunctionASTNode> parseFunction();
	std::unique_ptr<ASTNode> parseFactor();
	std::unique_ptr<ASTNode> parseTerm();
	std::unique_ptr<ASTNode> parseExpression();
	std::unique_ptr<VariableASTNode> parseVariable(std::string variableName);
	std::unique_ptr<PrototypeASTNode> parsePrototype();
	std::unique_ptr<CallASTNode> parseCall(std::string functionName);
};
