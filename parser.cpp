#include <iostream>
#include "ast.hpp"
#include "parser.hpp"

using namespace llvm;
using namespace llvm::sys;

void Parser::openSourceFile(char filePath[]) {
	lexer.openSourceFile(filePath);
}

void Parser::nextToken(){
	token = lexer.getToken();		
}

int Parser::accept(TokenType tokenType) {
//	std::cout << token.type << ", " << token.value << std::endl;
	if (token.type == tokenType) {
		nextToken();
		return 1;
	}	
	return 0;
}

int Parser::expect(TokenType tokenType){
	if (accept(tokenType)) {
		return 1;
	}
	return 0;
}

std::unique_ptr<ASTNode> Parser::parseFactor(){
	Token factorToken = token;
	if(accept(NUMBER_T)) {
		return std::make_unique<NumberExpressionNode>(factorToken);
	} else if (accept(IDENTIFIER_T)) {
		if (accept(OPEN_P_T)) {
			return parseCall(factorToken.value);
		} else {
			return std::make_unique<VariableExpressionNode>(factorToken);
		}
	} else if (accept(OPEN_P_T)) {
		auto expression = parseExpression();
		expect(CLOSED_P_T);
		return expression; 
	} else {
		std::cerr << "error in parser: error in parsing expression\n";
		return nullptr;
	}
}

std::unique_ptr<ASTNode> Parser::parseTerm(){
	auto expression = parseFactor();

	while (token.value[0] == '*' || token.value[0] == '/') {
		Token op = token; 
		nextToken();
		expression = std::make_unique<ExpressionNode>(op, std::move(expression), std::move(parseFactor())); 
	}
	return expression;
}

std::unique_ptr<ASTNode> Parser::parseExpression(){
	auto expression = parseTerm();

	while (token.value[0] == '+' || token.value[0] == '-'){
		Token op = token; 
		nextToken();
		expression = std::make_unique<ExpressionNode>(op, std::move(expression), std::move(parseExpression())); 
	}

	return std::move(expression);
}

std::unique_ptr<CallASTNode> Parser::parseCall(std::string functionName) {
	std::vector<std::unique_ptr<ASTNode>> Args;
	while (token.type != CLOSED_P_T) {
		auto arg = parseExpression();	
		Args.push_back(std::move(arg));
	}	
	if(!expect(CLOSED_P_T)) {
		std::cerr << "error in parser: expected closed parenthesis\n";
	}

	return std::make_unique<CallASTNode>(functionName, std::move(Args)); 
}

std::unique_ptr<VariableASTNode> Parser::parseVariable(std::string variableName) {
	if (auto body = parseExpression())
		return std::make_unique<VariableASTNode>(variableName, std::move(body));
	return nullptr;
}

std::unique_ptr<PrototypeASTNode> Parser::parsePrototype() {
	std::string functionName = token.value;	
	if (expect(IDENTIFIER_T)) {
		if (expect(OPEN_P_T)) {
			std::vector<std::string> arguments;
			while (token.type == IDENTIFIER_T) {
				arguments.push_back(token.value);
				nextToken(); 
			};
			if(!expect(CLOSED_P_T)) {
				std::cerr << "error in parser: expected closed parenthesis\n";
			}
			return std::make_unique<PrototypeASTNode>(functionName, std::move(arguments));
		} else {
			std::cerr << "error in parser: expected open parenthesis\n";
		}
	}
	std::cerr << "error in parser: expected identifier\n";
	return nullptr;
}

std::unique_ptr<FunctionASTNode> Parser::parseFunction(){
	auto proto = parsePrototype();
	if(!proto) {
		std::cerr << "error in parser: error in parsing prototype\n";
		return nullptr;
	}

	std::vector<std::unique_ptr<ASTNode>> body;
	std::unique_ptr<ASTNode> bodyNode = nullptr;
	do {
		Token saveToken = token;
		if (accept(DEFINITION_T)) {
			bodyNode = parseFunction();
			body.push_back(std::move(bodyNode));
		} else if (accept(IDENTIFIER_T)) {
			if (accept(OPEN_P_T)) {
				bodyNode = parseCall(saveToken.value);
				body.push_back(std::move(bodyNode));
			} else if (accept(ASSIGNMENT_T)) {
				bodyNode = parseVariable(saveToken.value);
				body.push_back(std::move(bodyNode));
			}
		}
	} while (token.type != RETURN_T);

	if (expect(RETURN_T)) {
		auto returnStatement = parseExpression();
		body.push_back(std::move(returnStatement));

		auto function = std::make_unique<FunctionASTNode>(std::move(proto), std::move(body));
		return std::move(function);
	}
	std::cerr << "error in parser: expected return\n";
	return nullptr;
}

std::unique_ptr<FunctionASTNode> Parser::parseProgram(){
	if (accept(DEFINITION_T)) {
		return parseFunction(); 
	}
	return nullptr;
}
