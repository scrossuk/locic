#ifndef LOCIC_AST_FUNCTION_HPP
#define LOCIC_AST_FUNCTION_HPP

#include <string>
#include <vector>
#include <Locic/AST/Scope.hpp>
#include <Locic/AST/Type.hpp>
#include <Locic/AST/TypeInstance.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST {

	struct Function{
		enum TypeEnum{
			DEFINITION,
			DECLARATION
		} typeEnum;
		
		bool isMethod, isVarArg;
		Type * returnType;
		std::string name;
		std::vector<TypeVar *> parameters;
		
		// NULL for declarations.
		Scope * scope;
		
		inline Function(TypeEnum e, bool vA, Type * t, const std::string& n, const std::vector<TypeVar*>& p, Scope * s)
			: typeEnum(e), isMethod(false),
			isVarArg(vA), returnType(t), name(n),
			parameters(p), scope(s) { }
		
		inline static Function * Decl(Type * returnType, const std::string& name, const std::vector<TypeVar*>& parameters){
			return new Function(DECLARATION, false, returnType, name, parameters, NULL);
		}
		
		inline static Function * VarArgDecl(Type * returnType, const std::string& name, const std::vector<TypeVar*>& parameters){
			return new Function(DECLARATION, true, returnType, name, parameters, NULL);
		}
		
		inline static Function * Def(Type * returnType, const std::string& name, const std::vector<TypeVar*>& parameters, Scope * scope){
			return new Function(DEFINITION, false, returnType, name, parameters, scope);
		}
		
		inline std::string getFullName() const{
			return name;
		}
	};
	
}

#endif
