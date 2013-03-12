#ifndef LOCIC_SEMANTICANALYSIS_NODE_HPP
#define LOCIC_SEMANTICANALYSIS_NODE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/Optional.hpp>

#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Node {
			public:
				enum Kind {
					NONE,
					NAMESPACE,
					TYPEINSTANCE,
					FUNCTION,
					TEMPLATEVAR,
					LOCALVAR
				};
				
				inline static Node None() {
					return Node(NONE);
				}
				
				inline static Node Namespace(AST::Namespace * ast, SEM::Namespace* sem) {
					Node node(NAMESPACE);
					node.data_->ast.nameSpace = ast;
					node.data_->sem.nameSpace = sem;
					return node;
				}
				
				inline static Node TypeInstance(AST::TypeInstance * ast, SEM::TypeInstance* sem) {
					Node node(TYPEINSTANCE);
					node.data_->ast.typeInstance = ast;
					node.data_->sem.typeInstance = sem;
					return node;
				}
				
				inline static Node Function(AST::Function * ast, SEM::Function* sem) {
					Node node(FUNCTION);
					node.data_->ast.function = ast;
					node.data_->sem.function = sem;
					return node;
				}
				
				inline static Node TemplateVar(AST::TemplateTypeVar * astVar, SEM::TemplateVar * semVar){
					Node node(TEMPLATEVAR);
					node.data_->ast.templateVar = ast;
					node.data_->sem.templateVar = sem;
					return node;
				}
				
				inline Kind kind() const {
					return data_->kind;
				}
				
				inline StringMap<Node>& children() {
					return data_->children;
				}
				
				inline const StringMap<Node>& children() const {
					return data_->children;
				}
				
				inline Node getChild(const std::string& name) const {
					return children().tryGet(name);
				}
				
				inline bool tryAttach(const std::string& name, Node node) {
					return children().tryInsert(name, Node);
				}
				
				inline void attach(const std::string& name, Node node) {
					children().insert(name, Node);
				}
				
				inline bool isNone() const {
					return kind() == NONE;
				}
				
				inline bool isNotNone() const {
					return kind() != NONE;
				}
				
				inline bool isFunction() const {
					return kind() == FUNCTION;
				}
				
				inline bool isNamespace() const {
					return kind() == NAMESPACE;
				}
				
				inline bool isTypeInstance() const {
					return kind() == TYPEINSTANCE;
				}
				
				inline SEM::Namespace* getSEMNamespace() const {
						assert(kind() == NAMESPACE);
						return data_->sem.nameSpace;
					}
					
				inline SEM::TypeInstance* getSEMTypeInstance() const {
						assert(kind() == TYPEINSTANCE);
						return data_->sem.typeInstance;
					}
				
				inline SEM::Function* getSEMFunction() const {
						assert(kind() == FUNCTION);
						return data_->sem.function;
					}
				
			private:
				struct NodeData{
					Kind kind;
					StringMap<Node> children;
					
					union {
						void* nullPtr;
						AST::Namespace* nameSpace;
						AST::TypeInstance* typeInstance;
						AST::Function* function;
					} ast;
				
					union {
						void* nullPtr;
						SEM::Namespace* nameSpace;
						SEM::TypeInstance* typeInstance;
						SEM::Function* function;
					} sem;
					
					inline NodeData(Kind k)
						: kind_(k), ast.nullPtr(NULL), sem.nullPtr(NULL){ }
				};
				
				inline Node(Kind k)
					: data_(new NodeData(k)) { }
				
				boost::shared_ptr<NodeData> data_;
				
		};
		
	}
	
}

#endif
