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
			makeClassDef(String name, AST::Node<AST::VarList> variables,
			             AST::Node<AST::FunctionList> methods,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeDatatype(String name, AST::Node<AST::VarList> variables,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeUnionDatatype(String name, AST::Node<AST::TypeInstanceList> variants,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstanceList>
			makeTypeInstanceList(AST::TypeInstanceList list,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeEnum(String name, AST::Node<AST::StringList> constructorList,
			         const Debug::SourcePosition& start);
			
			AST::Node<AST::StringList>
			makeStringList(AST::StringList stringList,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeException(String name, AST::Node<AST::VarList> variables,
			              AST::Node<AST::ExceptionInitializer> initializer,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::ExceptionInitializer>
			makeNoneExceptionInitializer(const Debug::SourcePosition& start);
			
			AST::Node<AST::ExceptionInitializer>
			makeExceptionInitializer(AST::Node<AST::Symbol> symbol,
			                         AST::Node<AST::ValueDeclList> valueList,
			                         const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeInterface(String name, AST::Node<AST::FunctionList> methods,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makePrimitive(String name, AST::Node<AST::FunctionList> methods,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeOpaqueStruct(String name, const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeStruct(String name, AST::Node<AST::VarList> variables,
			           const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeInstance>
			makeUnion(String name, AST::Node<AST::VarList> variables,
			          const Debug::SourcePosition& start);
			
			AST::Node<AST::FunctionList>
			makeFunctionList(AST::FunctionList functionList,
			                 const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif