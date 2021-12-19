# The Nice programming language 
Nice is a simple procedural programming language that supports functions, evaluating basic math expressions, and local variables

## Compiler Design
The design of the compiler is split into 3 main components:
- `lexer` performs lexical analysis (assigns each word or character in the source text a meaningful name).
- `parser` makes sure the source text conforms to the language grammar and generates an abstract syntax tree using a recursive descent parser. 
- `codegen` performs code generation to LLVM IR.


Also included are two helper classes:
- `ast` defines the abstract syntax tree nodes
- `tokens` defines the language tokens

## Examples using Nice
compiling using llvm-config to print necessary flags for LLVM:

clang++ -O3 parser.cpp AST.cpp codegen.cpp lexer.cpp main.cpp \`llvm-config --cxxflags --ldflags --system-libs --libs all`

	function main() 

		function example(a b c d)
	    		return (a * b) / (c + d)

	return example(10 20 5 5)

Compiling the above will output the following LLVM IR to `output.ll`.

	; ModuleID = 'Module'
	source_filename = "Module"

	define double @main() {
	entry:
		%calltmp = call double @example(double 1.000000e+01, double 2.000000e+01, double 5.000000e+00, double 5.000000e+00)
		ret double %calltmp
	}

	define double @example(double %a, double %b, double %c, double %d) {
	entry:
		%multmp = fmul double %a, %b
		%addtmp = fadd double %c, %d
		%divtmp = fdiv double %multmp, %addtmp
		ret double %divtmp
	}

## Resources
- https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html
- https://en.wikipedia.org/wiki/Recursive_descent_parser
