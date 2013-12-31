#ifndef LOCIC_AST_FUNCTION_HPP
#define LOCIC_AST_FUNCTION_HPP

#include <string>
#include <vector>
#include <locic/AST/Node.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		struct Function {
			enum TypeEnum {
				DEFAULTDEFINITION,
				DEFINITION,
				DECLARATION
			} typeEnum;
			
			bool isMethod, isVarArg;
			Node<Type> returnType;
			std::string name;
			Node<TypeVarList> parameters;
			
			// NULL for declarations.
			Node<Scope> scope;
			
			inline Function(TypeEnum e, bool isM, bool vA, Node<Type> t, const std::string& n, const Node<TypeVarList>& p, Node<Scope> s)
				: typeEnum(e), isMethod(isM),
				  isVarArg(vA), returnType(t), name(n),
				  parameters(p), scope(s) { }
				  
			inline static Function* Decl(Node<Type> returnType, const std::string& name, const Node<TypeVarList>& parameters) {
				return new Function(DECLARATION, false, false, returnType, name, parameters, Node<Scope>());
			}
			
			inline static Function* VarArgDecl(Node<Type> returnType, const std::string& name, const Node<TypeVarList>& parameters) {
				return new Function(DECLARATION, false, true, returnType, name, parameters, Node<Scope>());
			}
			
			inline static Function* Def(Node<Type> returnType, const std::string& name, const Node<TypeVarList>& parameters, Node<Scope> scope) {
				return new Function(DEFINITION, false, false, returnType, name, parameters, scope);
			}
			
			inline static Function* DefaultStaticDef(const std::string& name) {
				return new Function(DEFAULTDEFINITION, false, false, Node<Type>(), name, Node<TypeVarList>(), Node<Scope>());
			}
			
			inline static Function* DefaultMethodDef(const std::string& name) {
				return new Function(DEFAULTDEFINITION, true, false, Node<Type>(), name, Node<TypeVarList>(), Node<Scope>());
			}
			
			inline static Function* Destructor(Node<Scope> scope) {
				return new Function(DEFINITION, true, false, makeNode(scope.location(), Type::Void()), "__destructor", makeDefaultNode<TypeVarList>(), scope);
			}
			
			inline std::string toString() const {
				if (typeEnum == DEFAULTDEFINITION) {
					return makeString("DefaultFunction(name = %s)", name.c_str());
				} else {
					return makeString("Function(name = %s, returnType = %s, ... (TODO))", name.c_str(), returnType.toString().c_str());
				}
			}
		};
		
		typedef std::vector<Node<Function>> FunctionList;
		
	}
	
}

#endif
