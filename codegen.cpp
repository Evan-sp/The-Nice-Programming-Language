#include "codegen.hpp"
#include <iostream>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;
static std::map<std::string, AllocaInst* > NamedValues;

static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
static std::map<std::string, std::unique_ptr<PrototypeASTNode>> FunctionProtos;
static ExitOnError ExitOnErr;

void Codegen::InitializeModule() {
	// Open a new context and module.
	TheContext = std::make_unique<LLVMContext>();
	TheModule = std::make_unique<Module>("Module", *TheContext);

	// Create a new builder for the module.
	Builder = std::make_unique<IRBuilder<>>(*TheContext);

	// Create a new pass manager attached to it.
	TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());
	
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	// Promote allocas to registers.
	TheFPM->add(createPromoteMemoryToRegisterPass());
	TheFPM->add(createInstructionCombiningPass());
	// Reassociate expressions.
	TheFPM->add(createReassociatePass());
	// Eliminate Common SubExpressions.
	TheFPM->add(createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	TheFPM->add(createCFGSimplificationPass());
	
	TheFPM->doInitialization();
}

void Codegen::outputFiles() {
	TheModule->print(errs(), nullptr);
	std::error_code errorcode;
	auto stream = new llvm::raw_fd_ostream("output.ll", errorcode);
	TheModule->print(*stream, nullptr);
}

void Codegen::functionDefinition(std::unique_ptr<FunctionASTNode> FnAST) {
	if (auto *FnIR = FnAST->codegen()) {
		FnIR->print(errs());
	}
}

void Codegen::TopLevelExpression(std::unique_ptr<FunctionASTNode> FnAST) {
	if (auto *FnIR = FnAST->codegen()) {
		// Remove the anonymous expression.
		FnIR->eraseFromParent();
	}
}

Value* CallASTNode::codegen() {
	Function *Callee = TheModule->getFunction(callee);

	if (!Callee) {
		std::cerr << "error in code generation: unknown function reference\n";
		return nullptr;
	}
	if (Callee->arg_size() != Args.size()) {
		std::cerr << "error in code generation: incorrect number of arguments\n";
		return nullptr;
	}

	std::vector<Value *> ArgsV;
	for (unsigned i = 0, e = Args.size(); i != e; ++i) {
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back())
			return nullptr;
	}

	return Builder->CreateCall(Callee, ArgsV, "calltmp");
}


Value* NumberExpressionNode::codegen(){
	double numberValue = strtod(token.value.c_str(), nullptr);
	return ConstantFP::get(*TheContext, APFloat(numberValue));
}

Value* VariableExpressionNode::codegen(){
	AllocaInst *A = NamedValues[token.value];
	if (!A) {
		std::cerr << "error in code generation: variable not found\n";
		return nullptr;
	}

	return Builder->CreateLoad(A->getAllocatedType(), A, token.value.c_str());
}

Value* ExpressionNode::codegen(){
	Value *L = leftNode->codegen();
	Value *R = rightNode->codegen();
	if (!L || !R) {
		return nullptr;
	}

	switch (token.value[0]) {
		case '+':
			return Builder->CreateFAdd(L, R, "addtmp");
		case '-':
			return Builder->CreateFSub(L, R, "subtmp");
		case '*':
			return Builder->CreateFMul(L, R, "multmp");
		case '/':
			return Builder->CreateFDiv(L, R, "divtmp");
		case '<':
			L = Builder->CreateFCmpULT(L, R, "cmptmp");
			// Convert bool 0/1 to double 0.0 or 1.0
			return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
		default:
			return nullptr;
	}
}

static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, StringRef VarName) {
	IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
			TheFunction->getEntryBlock().begin());
	return TmpB.CreateAlloca(Type::getDoubleTy(*TheContext), nullptr, VarName);
}

Value* VariableASTNode::codegen() {
	/*
	//create llvm objects
//NamedValues[VarName] = Alloca;

	Value *BodyVal = body->codegen();
	if (!BodyVal)
	return nullptr;

	// Return the body computation.
	return BodyVal;
	*/
	/*
	Value *variableValue = NamedValues[name];
	return Builder->CreateLoad(variableValue, name.c_str());
	*/
	Value *val = body->codegen();
	Value *variable = NamedValues[name];
	if(!variable) {
		Function *TheFunction = Builder->GetInsertBlock()->getParent();
		
		AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, name);
	    	return Builder->CreateStore(val, Alloca);
	}
	Builder->CreateStore(val, variable);
	return val;
}

Function* PrototypeASTNode::codegen(){
	// Make the function type:  double(double,double) etc.
	std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
	//void return function:  FunctionType *FT = FunctionType::get(Type::getVoidTy(*TheContext), Doubles, false);
	FunctionType *FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto &Arg : F->args()) {
		Arg.setName(Args[Idx++]);
	}

	return F;
}

Function* FunctionASTNode::codegen() {
	// First, check for an existing function from a previous 'extern' declaration.
	Function *TheFunction = TheModule->getFunction(Proto->getName());

	if (!TheFunction)
		TheFunction = Proto->codegen();

	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
	Builder->SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	
	//here we clear the names values, save and restore old bindings
	std::map<std::string, AllocaInst* > OldBindings = NamedValues;

	NamedValues.clear();
	for (auto &Arg : TheFunction->args()) {
		// Create an alloca for this variable.
		AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

		// Store the initial value into the alloca.
		Builder->CreateStore(&Arg, Alloca);

		// Add arguments to variable symbol table.
		NamedValues[std::string(Arg.getName())] = Alloca;
	}

	Value *RetVal;
	for (auto &BodyNode : Body) {
		RetVal = BodyNode->codegen();
		Builder->SetInsertPoint(BB);

		//if (Value *RetVal = BodyNode->codegen()) {

		//printf call
		// auto formatVal = Builder->CreateGlobalStringPtr("%f");
		// std::vector<Value*> args;
		// args.push_back(formatVal);
		// args.push_back(RetVal);
		// Builder->CreateCall(printfFunc, args);
		//

		//}
	}
	NamedValues = OldBindings;

	// Finish off the function.
	Builder->CreateRet(RetVal);

	// Validate the generated code, checking for consistency.
	verifyFunction(*TheFunction);

	// Run the optimizer on the function.
	TheFPM->run(*TheFunction);

	return TheFunction;
}
