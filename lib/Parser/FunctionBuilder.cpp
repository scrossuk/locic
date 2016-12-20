#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/FunctionBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		FunctionBuilder::FunctionBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		FunctionBuilder::~FunctionBuilder() { }
		
		AST::Node<AST::Function>
		FunctionBuilder::makeFunctionDecl(bool isVarArg, bool isStatic,
		                                  AST::Node<AST::TypeDecl> returnType, AST::Node<Name> name,
		                                  AST::Node<AST::VarList> parameters,
		                                  AST::Node<AST::ConstSpecifier> constSpecifier,
		                                  AST::Node<AST::RequireSpecifier> noexceptSpecifier,
		                                  AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setIsVarArg(isVarArg);
			function->setIsStatic(isStatic);
			function->setNameDecl(std::move(name));
			function->setReturnType(std::move(returnType));
			function->setParameterDecls(std::move(parameters));
			function->setConstSpecifier(std::move(constSpecifier));
			function->setNoexceptSpecifier(std::move(noexceptSpecifier));
			function->setRequireSpecifier(std::move(requireSpecifier));
			return AST::makeNode(location, function.release());
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeFunctionDef(bool isVarArg, bool isStatic,
		                                 AST::Node<AST::TypeDecl> returnType, AST::Node<Name> name,
		                                 AST::Node<AST::VarList> parameters,
		                                 AST::Node<AST::ConstSpecifier> constSpecifier,
		                                 AST::Node<AST::RequireSpecifier> noexceptSpecifier,
		                                 AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                 AST::Node<AST::Scope> scope,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setIsVarArg(isVarArg);
			function->setIsStatic(isStatic);
			function->setNameDecl(std::move(name));
			function->setReturnType(std::move(returnType));
			function->setParameterDecls(std::move(parameters));
			function->setConstSpecifier(std::move(constSpecifier));
			function->setNoexceptSpecifier(std::move(noexceptSpecifier));
			function->setRequireSpecifier(std::move(requireSpecifier));
			function->setScope(std::move(scope));
			return AST::makeNode(location, function.release());
			
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeDefaultMethod(bool isStatic, AST::Node<Name> name,
		                                   AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                   const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setIsStatic(isStatic);
			function->setAutoGenerated(true);
			function->setNameDecl(std::move(name));
			function->setRequireSpecifier(std::move(requireSpecifier));
			return AST::makeNode(location, function.release());
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeDestructor(AST::Node<Name> name, AST::Node<AST::Scope> scope,
		                                const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setNameDecl(std::move(name));
			function->setReturnType(AST::makeNode(scope.location(),
			                                      AST::TypeDecl::Void()));
			function->setParameterDecls(AST::makeDefaultNode<AST::VarList>());
			function->setScope(std::move(scope));
			return AST::makeNode(location, function.release());
		}
		
		AST::Node<Name>
		FunctionBuilder::makeName(Name name, const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new Name(std::move(name)));
		}
		
	}
	
}
