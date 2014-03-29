#include <vector>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		// TODO: actually generate the code here (i.e. move it out of CodeGen).
		SEM::Function* CreateDefaultConstructor(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			const bool isStatic = true;
			const bool isConst = false;
			const auto functionType = SEM::Type::Function(isVarArg, typeInstance->selfType(), typeInstance->constructTypes());
			return SEM::Function::DefDefault(isStatic, isConst, functionType, typeInstance->name() + "Create");
		}
		
		bool HasDefaultImplicitCopy(SEM::TypeInstance* typeInstance) {
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasDefaultImplicitCopy(variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				for (auto var: typeInstance->variables()) {
					if (!supportsImplicitCopy(var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		// TODO: actually generate the code here (i.e. move it out of CodeGen).
		SEM::Function* CreateDefaultImplicitCopy(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			const bool isStatic = false;
			const bool isConst = true;
			const auto functionType = SEM::Type::Function(isVarArg, typeInstance->selfType(), {});
			return SEM::Function::DefDefault(isStatic, isConst, functionType, typeInstance->name() + "implicitCopy");
		}
		
		SEM::Function* CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance) {
			// TODO: fix location.
			const auto location = Debug::SourceLocation::Null();
			
			const bool isVarArg = false;
			const bool isMethod = true;
			const bool isStatic = false;
			const bool isConst = true;
			
			const auto selfType = typeInstance->selfType();
			const auto selfValue = SEM::Value::Self(SEM::Type::Reference(selfType)->createRefType(selfType));
			
			const auto intType = getBuiltInType(context, "int_t")->selfType();
			const auto functionType = SEM::Type::Function(isVarArg, intType, { selfType });
			
			const auto operandVar = SEM::Var::Basic(selfType, selfType);
			const auto operandValue = SEM::Value::LocalVar(operandVar);
			
			const auto functionScope = new SEM::Scope();
			
			if (typeInstance->isUnionDatatype()) {
				std::vector<SEM::SwitchCase*> switchCases;
				for (size_t i = 0; i < typeInstance->variants().size(); i++) {
					const auto variantTypeInstance = typeInstance->variants().at(i);
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					const auto caseVarValue = SEM::Value::LocalVar(caseVar);
					
					const auto caseScope = new SEM::Scope();
					
					std::vector<SEM::SwitchCase*> subSwitchCases;
					for (size_t j = 0; j < typeInstance->variants().size(); j++) {
						const auto subVariantTypeInstance = typeInstance->variants().at(j);
						const auto subVariantType = subVariantTypeInstance->selfType();
						const auto subCaseVar = SEM::Var::Basic(subVariantType, subVariantType);
						const auto subCaseVarValue = SEM::Value::LocalVar(subCaseVar);
						
						const auto subCaseScope = new SEM::Scope();
						const auto plusOneConstant = SEM::Value::Constant(Constant::Integer(1), intType);
						if (i < j) {
							const auto minusOneConstant = CallValue(GetMethod(plusOneConstant, "minus", location), {}, location);
							subCaseScope->statements().push_back(SEM::Statement::Return(minusOneConstant));
						} else if (i > j) {
							subCaseScope->statements().push_back(SEM::Statement::Return(plusOneConstant));
						} else {
							// TODO: Either there needs to be an implicit copy here,
							// or 'compare' should accept its argument by reference.
							const auto compareResult = CallValue(GetMethod(caseVarValue, "compare", location), { subCaseVarValue }, location);
							subCaseScope->statements().push_back(SEM::Statement::Return(compareResult));
						}
						
						subSwitchCases.push_back(new SEM::SwitchCase(subCaseVar, subCaseScope));
					}
					
					caseScope->statements().push_back(SEM::Statement::Switch(derefAll(operandValue), subSwitchCases));
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, caseScope));
				}
				
				functionScope->statements().push_back(SEM::Statement::Switch(derefAll(selfValue), switchCases));
			} else {
				auto currentScope = functionScope;
				
				for (const auto memberVar: typeInstance->variables()) {
					const auto selfMember = SEM::Value::MemberAccess(derefValue(selfValue), memberVar);
					const auto operandMember = SEM::Value::MemberAccess(derefValue(operandValue), memberVar);
					const auto compareResult = CallValue(GetMethod(selfMember, "compare", location), { operandMember }, location);
					const auto isZero = CallValue(GetMethod(compareResult, "isZero", location), {}, location);
					
					const auto ifTrueScope = new SEM::Scope();
					const auto ifFalseScope = new SEM::Scope();
					
					const auto ifStatement = SEM::Statement::If({ new SEM::IfClause(isZero, ifTrueScope) }, ifFalseScope);
					ifFalseScope->statements().push_back(SEM::Statement::Return(compareResult));
					currentScope->statements().push_back(ifStatement);
					
					currentScope = ifTrueScope;
				}
				
				const auto zeroConstant = SEM::Value::Constant(Constant::Integer(0), intType);
				currentScope->statements().push_back(SEM::Statement::Return(zeroConstant));
			}
			
			return SEM::Function::Def(isMethod, isStatic, isConst, functionType, typeInstance->name() + "compare", { operandVar }, functionScope);
		}
		
		SEM::Function* CreateDefaultMethod(Context&, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name) {
			if (isStatic && name.last() == "Create") {
				return CreateDefaultConstructor(typeInstance);
			} if (!isStatic && name.last() == "implicitCopy" && HasDefaultImplicitCopy(typeInstance)) {
				return CreateDefaultImplicitCopy(typeInstance);
			}
			
			throw ErrorException(makeString("%s method '%s' does not have a default implementation.",
				isStatic ? "Static" : "Non-static", name.toString().c_str()));
		}
		
	}
	
}

