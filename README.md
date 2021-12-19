# The Nice programming language 
Nice is a simple procedural programming language that supports functions, evaluating basic math expressions, and local variables

## Compiler design
The design of the compiler is split into 3 main components:
- `lexer` performs lexical analysis (assigns each word or character in the source text a meaningful name).
- `parser` makes sure the source text conforms to the language grammar and generates an abstract syntax tree using a recursive descent parser. 
- `codegen` performs code generation to LLVM IR.


Also included are two helper classes:
- `ast` defines the abstract syntax tree nodes
- `tokens` defines the language tokens

## Examples using Nice
compiling using llvm-config to print necessary flags:

`` clang++ -O3 parser.cpp AST.cpp codegen.cpp lexer.cpp main.cpp `llvm-config --cxxflags --ldflags --system-libs --libs all` ``

An example program: 

	function main() 

		function example(a b c d)
	    		return (a * b) / (c + d)

	return example(10 20 5 5)

Compiling the above will output LLVM IR to `output.ll`. This file can be further compiled to machine code with the LLVM compiler or executed with the LLVM interpreter.

## Resources
- https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html
- https://en.wikipedia.org/wiki/Recursive_descent_parser
