#ifndef AST_HPP
#define AST_HPP

#include "token.hpp"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"

class ASTNode {
public:
	virtual ~ASTNode() = default;
	virtual llvm::Value *codegen() = 0;
}; 

class ExpressionNode : public ASTNode {
public:
	std::unique_ptr<ASTNode> leftNode, rightNode;
	Token token;
	
	ExpressionNode(Token token, std::unique_ptr<ASTNode> leftNode, std::unique_ptr<ASTNode> rightNode)
		: token(token), leftNode(std::move(leftNode)), rightNode(std::move(rightNode)) {}

	llvm::Value *codegen() override;
};

class NumberExpressionNode : public ASTNode {
public:
	Token token;
	
	NumberExpressionNode(Token token) : token(token) {}

	llvm::Value *codegen() override;
};

class VariableExpressionNode : public ASTNode {
public:
	Token token;
	
	VariableExpressionNode(Token token) : token(token) {}

	llvm::Value *codegen() override;
};

class VariableASTNode : public ASTNode { 
	std::string name;
	std::unique_ptr<ASTNode> body;

public:
	VariableASTNode(const std::string &name, std::unique_ptr<ASTNode> body) 
		: name(name), body(std::move(body)) {}

	llvm::Value *codegen() override;
};

class PrototypeASTNode {
	std::string Name;
	std::vector<std::string> Args;
public:
	PrototypeASTNode(const std::string &Name, std::vector<std::string> Args)
		: Name(Name), Args(std::move(Args)) {}

	llvm::Function *codegen();
  	const std::string &getName() const { return Name; }
};

class FunctionASTNode : public ASTNode {

public:
  	std::unique_ptr<PrototypeASTNode> Proto;
	std::vector<std::unique_ptr<ASTNode>> Body;
  	FunctionASTNode(std::unique_ptr<PrototypeASTNode> Proto, std::vector<std::unique_ptr<ASTNode>> Body)
      		: Proto(std::move(Proto)), Body(std::move(Body)) {}

	llvm::Function *codegen() override;
};

class CallASTNode : public ASTNode {
	std::string callee;
	std::vector<std::unique_ptr<ASTNode>> Args;
public:
	CallASTNode(const std::string &callee, std::vector<std::unique_ptr<ASTNode>> Args)
		: callee(callee), Args(std::move(Args)) {}

	llvm::Value *codegen() override;
};

class AST {
public:
//	void printAST(ASTNode *node);
//	ASTNode* makeNode(Token token, ASTNode* left, ASTNode* right);
};

#endif
