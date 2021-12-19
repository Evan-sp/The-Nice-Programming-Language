#include "codegen.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]){
	Parser parser;
	parser.openSourceFile(argv[1]);
	parser.nextToken();
	auto main = parser.parseProgram();

	Codegen codegen;
	codegen.InitializeModule();
	main->codegen();
	codegen.outputFiles();
	return 0;
}

