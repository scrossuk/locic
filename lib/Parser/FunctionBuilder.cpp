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
		                                  AST::Node<AST::Type> returnType, AST::Node<Name> name,
		                                  AST::Node<AST::TypeVarList> parameters,
		                                  AST::Node<AST::ConstSpecifier> constSpecifier,
		                                  AST::Node<AST::RequireSpecifier> noexceptSpecifier,
		                                  AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto function = AST::Function::Decl(isVarArg, isStatic,
			                                          returnType, name,
			                                          parameters,
			                                          constSpecifier,
			                                          noexceptSpecifier,
			                                          requireSpecifier);
			return AST::makeNode(location, function);
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeFunctionDef(bool isVarArg, bool isStatic,
		                                 AST::Node<AST::Type> returnType, AST::Node<Name> name,
		                                 AST::Node<AST::TypeVarList> parameters,
		                                 AST::Node<AST::ConstSpecifier> constSpecifier,
		                                 AST::Node<AST::RequireSpecifier> noexceptSpecifier,
		                                 AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                 AST::Node<AST::Scope> scope,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto function = AST::Function::Def(isVarArg, isStatic,
			                                         returnType, name,
			                                         parameters, scope,
			                                         constSpecifier,
			                                         noexceptSpecifier,
			                                         requireSpecifier);
			return AST::makeNode(location, function);
			
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeDefaultMethod(bool isStatic, AST::Node<Name> name,
		                                   AST::Node<AST::RequireSpecifier> requireSpecifier,
		                                   const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			if (isStatic) {
				const auto function =
				    AST::Function::DefaultStaticMethodDef(name, requireSpecifier);
				return AST::makeNode(location, function);
			} else {
				const auto function =
				    AST::Function::DefaultMethodDef(name, requireSpecifier);
				return AST::makeNode(location, function);
			}
		}
		
		AST::Node<AST::Function>
		FunctionBuilder::makeDestructor(AST::Node<Name> name, AST::Node<AST::Scope> scope,
		                                const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto function = AST::Function::Destructor(name, scope);
			return AST::makeNode(location, function);
		}
		
		AST::Node<Name>
		FunctionBuilder::makeName(Name name, const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new Name(std::move(name)));
		}
		
	}
	
}
