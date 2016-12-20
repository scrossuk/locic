#include <locic/AST.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceBuilder.hpp>

namespace locic {
	
	namespace Parser {
		
		TypeInstanceBuilder::TypeInstanceBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		TypeInstanceBuilder::~TypeInstanceBuilder() { }
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeTypeInstanceNode(AST::TypeInstance* const typeInstance,
		                                          const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, typeInstance);
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeClassDecl(String name, AST::Node<AST::FunctionList> methods,
		                                   const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::CLASSDECL),
			                                 start);
			node->functionDecls = std::move(methods);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeClassDef(String name, AST::Node<AST::VarList> variables,
		                                  AST::Node<AST::FunctionList> methods,
		                                  const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::CLASSDEF),
			                                 start);
			node->variableDecls = std::move(variables);
			node->functionDecls = std::move(methods);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeDatatype(String name, AST::Node<AST::VarList> variables,
		                                  const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::DATATYPE),
			                                 start);
			node->variableDecls = std::move(variables);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeUnionDatatype(String name, AST::Node<AST::TypeInstanceList> variants,
		                                       const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::UNION_DATATYPE),
			                                 start);
			node->variantDecls = std::move(variants);
			return node;
		}
		
		AST::Node<AST::TypeInstanceList>
		TypeInstanceBuilder::makeTypeInstanceList(AST::TypeInstanceList list,
		                                          const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::TypeInstanceList(std::move(list)));
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeEnum(String name, AST::Node<AST::StringList> constructorList,
		                              const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::ENUM),
			                                 start);
			node->constructors = std::move(constructorList);
			return node;
		}
		
		AST::Node<AST::StringList>
		TypeInstanceBuilder::makeStringList(AST::StringList stringList,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::StringList(std::move(stringList)));
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeException(String name, AST::Node<AST::VarList> variables,
		                                   AST::Node<AST::ExceptionInitializer> initializer,
		                                   const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::EXCEPTION),
			                                 start);
			node->variableDecls = std::move(variables);
			node->initializer = std::move(initializer);
			return node;
		}
		
		AST::Node<AST::ExceptionInitializer>
		TypeInstanceBuilder::makeNoneExceptionInitializer(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ExceptionInitializer::None());
		}
		
		AST::Node<AST::ExceptionInitializer>
		TypeInstanceBuilder::makeExceptionInitializer(AST::Node<AST::Symbol> symbol,
		                                              AST::Node<AST::ValueDeclList> valueList,
		                                              const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ExceptionInitializer::Initialize(std::move(symbol), std::move(valueList)));
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeInterface(String name, AST::Node<AST::FunctionList> methods,
		                                   const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::INTERFACE),
			                                 start);
			node->functionDecls = std::move(methods);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makePrimitive(String name, AST::Node<AST::FunctionList> methods,
		                                   const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::PRIMITIVE),
			                                 start);
			node->functionDecls = std::move(methods);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeOpaqueStruct(String name, const Debug::SourcePosition& start) {
			return makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::OPAQUE_STRUCT),
			                            start);
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeStruct(String name, AST::Node<AST::VarList> variables,
		                                const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::STRUCT),
			                                 start);
			node->variableDecls = std::move(variables);
			return node;
		}
		
		AST::Node<AST::TypeInstance>
		TypeInstanceBuilder::makeUnion(String name, AST::Node<AST::VarList> variables,
		                               const Debug::SourcePosition& start) {
			auto node = makeTypeInstanceNode(new AST::TypeInstance(name, AST::TypeInstance::UNION),
			                                 start);
			node->variableDecls = std::move(variables);
			return node;
		}
		
		AST::Node<AST::FunctionList>
		TypeInstanceBuilder::makeFunctionList(AST::FunctionList functionList,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::FunctionList(std::move(functionList)));
		}
		
	}
	
}
