#ifndef LOCIC_PARSER_TYPEINSTANCEBUILDER_HPP
#define LOCIC_PARSER_TYPEINSTANCEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeInstanceBuilder {
		public:
			TypeInstanceBuilder(const TokenReader& reader);
			~TypeInstanceBuilder();
			
			AST::Node<AST::TypeInstance>
			makeTypeInstanceNode(AST::TypeInstance* typeInstance,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeClassDecl(String name, AST::Node<AST::FunctionList> methods,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeClassDef(String name, AST::Node<AST::TypeVarList> variables,
			             AST::Node<AST::FunctionList> methods,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeDatatype(String name, AST::Node<AST::TypeVarList> variables,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeUnionDatatype(String name, AST::Node<AST::TypeInstanceList> variants,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstanceList>
			makeTypeInstanceList(AST::TypeInstanceList list,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeException(String name, AST::Node<AST::TypeVarList> variables,
			              AST::Node<AST::ExceptionInitializer> initializer,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeInterface(String name, AST::Node<AST::FunctionList> methods,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeOpaqueStruct(String name, const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeStruct(String name, AST::Node<AST::TypeVarList> variables,
			           const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeUnion(String name, AST::Node<AST::TypeVarList> variables,
			          const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif