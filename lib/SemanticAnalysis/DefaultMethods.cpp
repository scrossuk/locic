#include <vector>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateDefaultConstructor(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			
			std::vector<SEM::Type*> templateVars;
			
			// The parent class type needs to include the template arguments.
			for (auto templateVar: typeInstance->templateVariables()) {
				templateVars.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
			
			auto returnType = SEM::Type::Object(typeInstance, templateVars);
			
			auto functionType = SEM::Type::Function(isVarArg, returnType, typeInstance->constructTypes());
			
			const bool isStatic = true;
			
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
					if (!var->constructType()->supportsImplicitCopy()) {
						return false;
					}
				}
				
				return true;
			}
		}
		
		SEM::Function* CreateDefaultImplicitCopy(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			
			std::vector<SEM::Type*> templateVars;
			
			// The parent class type needs to include the template arguments.
			for (auto templateVar: typeInstance->templateVariables()) {
				templateVars.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
				
			auto returnType = SEM::Type::Object(typeInstance, templateVars);
			
			auto functionType = SEM::Type::Function(isVarArg, returnType, std::vector<SEM::Type*>());
			
			const bool isStatic = false;
			
			return SEM::Function::DefDefault(isStatic, functionType, typeInstance->name() + "implicitCopy");
		}
		
		SEM::Function* CreateDefaultMethod(SEM::TypeInstance* typeInstance, bool isStatic, const Name& name) {
			if (isStatic && name.last() == "Create") {
				return CreateDefaultConstructor(typeInstance);
			} if (!isStatic && name.last() == "implicitCopy" && HasDefaultImplicitCopy(typeInstance)) {
				return CreateDefaultImplicitCopy(typeInstance);
			}
			
			throw TodoException(makeString("%s method '%s' does not have a default implementation.",
				isStatic ? "Static" : "Non-static", name.toString().c_str()));
		}
		
	}
	
}

