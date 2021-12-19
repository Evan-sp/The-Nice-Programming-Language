#include "ast.hpp"
class Codegen {
public:
	void InitializeModule(); 
	void outputFiles();
	void TopLevelExpression(std::unique_ptr<FunctionASTNode> FnAST);
	void functionDefinition(std::unique_ptr<FunctionASTNode> FnAST); 
};
