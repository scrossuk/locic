#ifndef LOCIC_SEMANTICANALYSIS_NODE_HPP
#define LOCIC_SEMANTICANALYSIS_NODE_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include <locic/Map.hpp>
#include <locic/Name.hpp>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>

namespace locic {

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
					VARIABLE,
					SWITCHCASE,
					CATCHCLAUSE
				};
				
				inline static Node None() {
					return Node(NONE);
				}
				
				inline static Node Namespace(const AST::NamespaceList& ast, SEM::Namespace* sem) {
					Node node(NAMESPACE);
					node.data_->ast.nameSpaceList = ast;
					node.data_->sem.nameSpace = sem;
					return node;
				}
				
				inline static Node TypeInstance(const AST::Node<AST::TypeInstance>& ast, SEM::TypeInstance* sem) {
					Node node(TYPEINSTANCE);
					node.data_->ast.typeInstance = ast;
					node.data_->sem.typeInstance = sem;
					return node;
				}
				
				inline static Node Function(const AST::Node<AST::Function>& ast, SEM::Function* sem) {
					Node node(FUNCTION);
					node.data_->ast.function = ast;
					node.data_->sem.function = sem;
					return node;
				}
				
				inline static Node TemplateVar(const AST::Node<AST::TemplateTypeVar>& ast, SEM::TemplateVar * sem){
					Node node(TEMPLATEVAR);
					node.data_->ast.templateVar = ast;
					node.data_->sem.templateVar = sem;
					return node;
				}
				
				inline static Node Scope(const AST::Node<AST::Scope>& ast, SEM::Scope * sem){
					Node node(SCOPE);
					node.data_->ast.scope = ast;
					node.data_->sem.scope = sem;
					return node;
				}
				
				inline static Node Variable(const AST::Node<AST::TypeVar>& ast, SEM::Var * sem){
					Node node(VARIABLE);
					node.data_->ast.var = ast;
					node.data_->sem.var = sem;
					return node;
				}
				
				inline static Node SwitchCase(const AST::Node<AST::SwitchCase>& ast, SEM::SwitchCase * sem){
					Node node(SWITCHCASE);
					node.data_->ast.switchCase = ast;
					node.data_->sem.switchCase = sem;
					return node;
				}
				
				inline static Node CatchClause(const AST::Node<AST::CatchClause>& ast, SEM::CatchClause * sem){
					Node node(CATCHCLAUSE);
					node.data_->ast.catchClause = ast;
					node.data_->sem.catchClause = sem;
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
				
				inline bool isSwitchCase() const {
					return kind() == SWITCHCASE;
				}
				
				inline bool isCatchClause() const {
					return kind() == CATCHCLAUSE;
				}
				
				inline AST::NamespaceList& getASTNamespaceList() {
					assert(isNamespace());
					return data_->ast.nameSpaceList;
				}
				
				inline const AST::NamespaceList& getASTNamespaceList() const {
					assert(isNamespace());
					return data_->ast.nameSpaceList;
				}
				
				inline const AST::Node<AST::TypeInstance>& getASTTypeInstance() const {
					assert(isTypeInstance());
					return data_->ast.typeInstance;
				}
				
				inline const AST::Node<AST::Function>& getASTFunction() const {
					assert(isFunction());
					return data_->ast.function;
				}
				
				inline const AST::Node<AST::TemplateTypeVar>& getASTTemplateVar() const {
					assert(isTemplateVar());
					return data_->ast.templateVar;
				}
				
				inline const AST::Node<AST::Scope>& getASTScope() const {
					assert(isScope());
					return data_->ast.scope;
				}
				
				inline const AST::Node<AST::SwitchCase>& getASTSwitchCase() const {
					assert(isSwitchCase());
					return data_->ast.switchCase;
				}
				
				inline SEM::Namespace* getSEMNamespace() const {
					assert(isNamespace());
					return data_->sem.nameSpace;
				}
					
				inline SEM::TypeInstance* getSEMTypeInstance() const {
					assert(isTypeInstance());
					return data_->sem.typeInstance;
				}
				
				inline SEM::Function* getSEMFunction() const {
					assert(isFunction());
					return data_->sem.function;
				}
				
				inline SEM::TemplateVar* getSEMTemplateVar() const {
					assert(isTemplateVar());
					return data_->sem.templateVar;
				}
				
				inline SEM::Scope* getSEMScope() const {
					assert(isScope());
					return data_->sem.scope;
				}
				
				inline SEM::Var* getSEMVar() const {
					assert(isVariable());
					return data_->sem.var;
				}
				
				inline SEM::SwitchCase* getSEMSwitchCase() const {
					assert(isSwitchCase());
					return data_->sem.switchCase;
				}
				
				inline SEM::CatchClause* getSEMCatchClause() const {
					assert(isCatchClause());
					return data_->sem.catchClause;
				}
				
				inline std::string toString() const {
					switch(kind()){
						case NONE:
							return "Node[None]()";
						case NAMESPACE:
							return makeString("Node[Namespace](%s)",
								getSEMNamespace()->name().c_str());
						case TYPEINSTANCE:
							return makeString("Node[TypeInstance](%s)",
								getSEMTypeInstance()->toString().c_str());
						case FUNCTION:
							return makeString("Node[Function](%s)",
								getSEMFunction()->toString().c_str());
						case TEMPLATEVAR:
							return makeString("Node[TemplateVar](%s)",
								getASTTemplateVar()->name.c_str());
						case SCOPE:
							return "Node[Scope]()";
						case VARIABLE:
							return "Node[Variable]()";
						case SWITCHCASE:
							return makeString("Node[SwitchCase](%s)",
								getSEMSwitchCase()->toString().c_str());
						case CATCHCLAUSE:
							return makeString("Node[CatchClause](%s)",
								getSEMCatchClause()->toString().c_str());
						default:
							assert(false && "Unknown node type.");
							return "Node([INVALID])";
					}
				}
				
			private:
				struct NodeData{
					Kind kind;
					StringMap<Node> children;
					
					// Can't use 'union' since members have
					// destructors.
					struct {
						AST::NamespaceList nameSpaceList;
						AST::Node<AST::TypeInstance> typeInstance;
						AST::Node<AST::Function> function;
						AST::Node<AST::TemplateTypeVar> templateVar;
						AST::Node<AST::Scope> scope;
						AST::Node<AST::TypeVar> var;
						AST::Node<AST::SwitchCase> switchCase;
						AST::Node<AST::CatchClause> catchClause;
					} ast;
				
					union {
						SEM::Namespace* nameSpace;
						SEM::TypeInstance* typeInstance;
						SEM::Function* function;
						SEM::TemplateVar* templateVar;
						SEM::Scope* scope;
						SEM::Var* var;
						SEM::SwitchCase* switchCase;
						SEM::CatchClause* catchClause;
					} sem;
					
					inline NodeData(Kind k)
						: kind(k) { }
				};
				
				inline Node(Kind k)
					: data_(new NodeData(k)) { }
				
				boost::shared_ptr<NodeData> data_;
				
		};
		
	}
	
}

#endif
