#include <vector>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		// TODO: actually generate the code here (i.e. move it out of CodeGen).
		SEM::Function* CreateDefaultConstructor(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			const bool isStatic = true;
			const auto functionType = SEM::Type::Function(isVarArg, typeInstance->selfType(), typeInstance->constructTypes());
			return SEM::Function::DefDefault(isStatic, functionType, typeInstance->name() + "Create");
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
			const auto functionType = SEM::Type::Function(isVarArg, typeInstance->selfType(), {});
			return SEM::Function::DefDefault(isStatic, functionType, typeInstance->name() + "implicitCopy");
		}
		
		// TODO: actually generate the code here (i.e. move it out of CodeGen).
		SEM::Function* CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			const bool isStatic = false;
			const auto intType = getBuiltInType(context, "int_t");
			const auto functionType = SEM::Type::Function(isVarArg, intType->selfType(), { typeInstance->selfType() });
			return SEM::Function::DefDefault(isStatic, functionType, typeInstance->name() + "compare");
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

