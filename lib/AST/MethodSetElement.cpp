#include <locic/AST/MethodSetElement.hpp>

#include <assert.h>

#include <algorithm>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/Predicate.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Support/MakeString.hpp>

namespace locic {

	namespace AST {
		
		MethodSetElement::MethodSetElement(
				TemplateVarArray argTemplateVariables,
				Predicate argConstPredicate,
				Predicate argNoexceptPredicate,
				Predicate argRequirePredicate,
				const bool argIsStatic,
				const Type* const argReturnType,
				TypeArray argParameterTypes
			)
			: templateVariables_(std::move(argTemplateVariables)),
			constPredicate_(std::move(argConstPredicate)),
			noexceptPredicate_(std::move(argNoexceptPredicate)),
			requirePredicate_(std::move(argRequirePredicate)),
			isStatic_(argIsStatic),
			returnType_(argReturnType),
			parameterTypes_(std::move(argParameterTypes)) { }
		
		MethodSetElement MethodSetElement::copy() const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(),
				noexceptPredicate().copy(),
				requirePredicate().copy(),
				isStatic(), returnType(), parameterTypes().copy());
		}
		
		MethodSetElement MethodSetElement::withRequirement(Predicate requirement) const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(),
				noexceptPredicate().copy(),
				Predicate::And(requirePredicate().copy(), std::move(requirement)),
				isStatic(), returnType(), parameterTypes().copy());
		}
		
		MethodSetElement MethodSetElement::withNoExceptPredicate(Predicate newNoExceptPredicate) const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(),
				std::move(newNoExceptPredicate),
				requirePredicate().copy(),
				isStatic(), returnType(), parameterTypes().copy());
		}
		
		const TemplateVarArray& MethodSetElement::templateVariables() const {
			return templateVariables_;
		}
		
		const Predicate& MethodSetElement::constPredicate() const {
			return constPredicate_;
		}
		
		const Predicate& MethodSetElement::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		const Predicate& MethodSetElement::requirePredicate() const {
			return requirePredicate_;
		}
		
		bool MethodSetElement::isStatic() const {
			return isStatic_;
		}
		
		const Type* MethodSetElement::returnType() const {
			return returnType_;
		}
		
		const TypeArray& MethodSetElement::parameterTypes() const {
			return parameterTypes_;
		}
		
		FunctionType MethodSetElement::createFunctionType(const bool isTemplated) const {
			const bool isVarArg = false;
			const bool isMethod = !isStatic();
			FunctionAttributes attributes(isVarArg, isMethod, isTemplated, noexceptPredicate().copy());
			return FunctionType(std::move(attributes), returnType(), parameterTypes().copy());
		}
		
		std::size_t MethodSetElement::hash() const {
			std::size_t seed = 0;
			
			boost::hash_combine(seed, templateVariables().hash());
			boost::hash_combine(seed, constPredicate().hash());
			boost::hash_combine(seed, noexceptPredicate().hash());
			boost::hash_combine(seed, requirePredicate().hash());
			boost::hash_combine(seed, isStatic());
			boost::hash_combine(seed, returnType());
			
			for (const auto& parameterType: parameterTypes()) {
				boost::hash_combine(seed, parameterType);
			}
			
			return seed;
		}
		
		bool MethodSetElement::operator==(const MethodSetElement& methodSetElement) const {
			return templateVariables() == methodSetElement.templateVariables() &&
				constPredicate() == methodSetElement.constPredicate() &&
				noexceptPredicate() == methodSetElement.noexceptPredicate() &&
				requirePredicate() == methodSetElement.requirePredicate() &&
				returnType() == methodSetElement.returnType() &&
				parameterTypes() == methodSetElement.parameterTypes() &&
				isStatic() == methodSetElement.isStatic();
		}
		
		std::string MethodSetElement::toString() const {
			return makeString("MethodSetElement(constPredicate: %s, noexceptPredicate: %s, requirePredicate: %s, returnType: %s, ...)",
				constPredicate().toString().c_str(),
				noexceptPredicate().toString().c_str(),
				requirePredicate().toString().c_str(),
				returnType()->toString().c_str());
		}
		
	}
	
}

