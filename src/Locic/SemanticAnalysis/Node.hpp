#ifndef LOCIC_SEMANTICANALYSIS_NODE_HPP
#define LOCIC_SEMANTICANALYSIS_NODE_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include <Locic/Map.hpp>
#include <Locic/Name.hpp>

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>

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
					SCOPE,
					VARIABLE
				};
				
				inline static Node None() {
					return Node(NONE);
				}
				
				inline static Node Namespace(AST::Namespace * ast, SEM::Namespace* sem) {
					Node node(NAMESPACE);
					node.data_->ast.nameSpace = ast;
					node.data_->sem= sem;
					return node;
				}
				
				inline static Node TypeInstance(AST::TypeInstance * ast, SEM::TypeInstance* sem) {
					Node node(TYPEINSTANCE);
					node.data_->ast.typeInstance = ast;
					node.data_->sem = sem;
					return node;
				}
				
				inline static Node Function(AST::Function * ast, SEM::Function* sem) {
					Node node(FUNCTION);
					node.data_->ast.function = ast;
					node.data_->sem = sem;
					return node;
				}
				
				inline static Node TemplateVar(AST::TemplateTypeVar * ast, SEM::TemplateVar * sem){
					Node node(TEMPLATEVAR);
					node.data_->ast.templateVar = ast;
					node.data_->sem = sem;
					return node;
				}
				
				inline static Node Scope(AST::Scope * ast, SEM::Scope * sem){
					Node node(SCOPE);
					node.data_->ast.scope = ast;
					node.data_->sem = sem;
					return node;
				}
				
				inline static Node Variable(AST::TypeVar * ast, SEM::Var * sem){
					Node node(VARIABLE);
					node.data_->ast.var = ast;
					node.data_->sem = sem;
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
					Optional<Node> child = children().tryGet(name);
					return child.hasValue() ? child.getValue() : Node::None();
				}
				
				inline bool tryAttach(const std::string& name, const Node& node) {
					return children().tryInsert(name, node);
				}
				
				inline void attach(const std::string& name, const Node& node) {
					children().insert(name, node);
				}
				
				inline void forceAttach(const std::string& name, const Node& node) {
					children().forceInsert(name, node);
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
				
				inline bool isTemplateVar() const {
					return kind() == TEMPLATEVAR;
				}
				
				inline bool isScope() const {
					return kind() == SCOPE;
				}
				
				inline bool isVariable() const {
					return kind() == VARIABLE;
				}
				
				inline AST::Namespace* getASTNamespace() const {
						assert(kind() == NAMESPACE);
						return data_->ast.nameSpace;
					}
					
				inline AST::TypeInstance* getASTTypeInstance() const {
						assert(kind() == TYPEINSTANCE);
						return data_->ast.typeInstance;
					}
				
				inline AST::Function* getASTFunction() const {
						assert(kind() == FUNCTION);
						return data_->ast.function;
					}
				
				inline AST::TemplateTypeVar* getASTTemplateVar() const {
						assert(kind() == TEMPLATEVAR);
						return data_->ast.templateVar;
					}
				
				inline AST::Scope* getASTScope() const {
						assert(isScope());
						return data_->ast.scope;
					}
				
				inline SEM::Object* getSEMObject() const {
					assert(isNotNone());
					assert(data_->sem != NULL);
					return data_->sem;
				}
				
				inline SEM::Namespace* getSEMNamespace() const {
						assert(kind() == NAMESPACE);
						return (SEM::Namespace*) data_->sem;
					}
					
				inline SEM::TypeInstance* getSEMTypeInstance() const {
						assert(kind() == TYPEINSTANCE);
						return (SEM::TypeInstance*) data_->sem;
					}
				
				inline SEM::Function* getSEMFunction() const {
						assert(kind() == FUNCTION);
						return (SEM::Function*) data_->sem;
					}
				
				inline SEM::TemplateVar* getSEMTemplateVar() const {
						assert(kind() == TEMPLATEVAR);
						return (SEM::TemplateVar*) data_->sem;
					}
				
				inline SEM::Scope* getSEMScope() const {
						assert(isScope());
						return (SEM::Scope*) data_->sem;
					}
				
				inline SEM::Var* getSEMVar() const {
					assert(isVariable());
					return (SEM::Var*) data_->sem;
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
						AST::TemplateTypeVar * templateVar;
						AST::Scope* scope;
						AST::TypeVar* var;
					} ast;
				
					SEM::Object* sem;
					
					inline NodeData(Kind k)
						: kind(k){
							ast.nullPtr = NULL;
							sem = NULL;
						}
				};
				
				inline Node(Kind k)
					: data_(new NodeData(k)) { }
				
				boost::shared_ptr<NodeData> data_;
				
		};
		
	}
	
}

#endif
