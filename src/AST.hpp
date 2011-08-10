#ifndef LOCIC_AST_HPP
#define LOCIC_AST_HPP

#include <string>

namespace Locic{

	class AST{
		public:
			virtual void print() = 0;
			
			inline virtual ~AST(){ }
			
	};
	
}

#include <Locic/AST/Add.hpp>
#include <Locic/AST/AddressOf.hpp>
#include <Locic/AST/AutoVarDeclaration.hpp>
#include <Locic/AST/Boolean.hpp>
#include <Locic/AST/ClassDeclaration.hpp>
#include <Locic/AST/ClassDefinition.hpp>
#include <Locic/AST/ClassMethodDeclaration.hpp>
#include <Locic/AST/ClassMethodDefinition.hpp>
#include <Locic/AST/ClassVar.hpp>
#include <Locic/AST/Construct.hpp>
#include <Locic/AST/Deref.hpp>
#include <Locic/AST/Divide.hpp>
#include <Locic/AST/File.hpp>
#include <Locic/AST/GetMemberVar.hpp>
#include <Locic/AST/If.hpp>
#include <Locic/AST/IfElse.hpp>
#include <Locic/AST/IsEqual.hpp>
#include <Locic/AST/List.hpp>
#include <Locic/AST/MemberVar.hpp>
#include <Locic/AST/MethodCall.hpp>
#include <Locic/AST/Multiply.hpp>
#include <Locic/AST/NamedConstruct.hpp>
#include <Locic/AST/Negate.hpp>
#include <Locic/AST/ParamSpec.hpp>
#include <Locic/AST/Return.hpp>
#include <Locic/AST/Scope.hpp>
#include <Locic/AST/SetMemberVar.hpp>
#include <Locic/AST/SetVar.hpp>
#include <Locic/AST/Subtract.hpp>
#include <Locic/AST/Ternary.hpp>
#include <Locic/AST/Type.hpp>
#include <Locic/AST/UnaryMinus.hpp>
#include <Locic/AST/UnaryPlus.hpp>
#include <Locic/AST/Value.hpp>
#include <Locic/AST/Var.hpp>
#include <Locic/AST/VarDeclaration.hpp>

#endif
