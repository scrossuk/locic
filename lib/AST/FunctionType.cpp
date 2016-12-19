#include <locic/AST/FunctionType.hpp>

#include <string>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeArray.hpp>

#include <locic/AST/Context.hpp>
#include <locic/SEM/Predicate.hpp>

#include <locic/Support/Hasher.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		FunctionAttributes::FunctionAttributes(const bool argIsVarArg,
		                                       const bool argIsMethod,
		                                       const bool argIsTemplated,
		                                       SEM::Predicate argNoExceptPredicate)
		: isVarArg_(argIsVarArg),
		isMethod_(argIsMethod),
		isTemplated_(argIsTemplated),
		noExceptPredicate_(std::move(argNoExceptPredicate)) { }
		
		FunctionAttributes FunctionAttributes::copy() const {
			return FunctionAttributes(isVarArg(),
			                          isMethod(),
			                          isTemplated(),
			                          noExceptPredicate().copy());
		}
		
		bool FunctionAttributes::isVarArg() const {
			return isVarArg_;
		}
		
		bool FunctionAttributes::isMethod() const {
			return isMethod_;
		}
		
		bool FunctionAttributes::isTemplated() const {
			return isTemplated_;
		}
		
		const SEM::Predicate& FunctionAttributes::noExceptPredicate() const {
			return noExceptPredicate_;
		}
		
		std::string FunctionAttributes::toString() const {
			return makeString("FunctionAttributes("
			                  "isVarArg: %s, "
			                  "isMethod: %s, "
			                  "isTemplated: %s, "
			                  "noExceptPredicate: %s)",
			                  isVarArg() ? "true" : "false",
			                  isMethod() ? "true" : "false",
			                  isTemplated() ? "true" : "false",
			                  noExceptPredicate().toString().c_str());
		}
		
		std::size_t FunctionAttributes::hash() const {
			Hasher hasher;
			hasher.add(isVarArg());
			hasher.add(isMethod());
			hasher.add(isTemplated());
			hasher.add(noExceptPredicate());
			return hasher.get();
		}
		
		bool FunctionAttributes::operator==(const FunctionAttributes& other) const {
			return isVarArg() == other.isVarArg() &&
			       isMethod() == other.isMethod() &&
			       isTemplated() == other.isTemplated() &&
			       noExceptPredicate() == other.noExceptPredicate();
		}
		
		FunctionTypeData::FunctionTypeData(FunctionAttributes argAttributes,
		                                   const AST::Type* const argReturnType,
		                                   AST::TypeArray argParameterTypes)
		: attributes_(std::move(argAttributes)),
		returnType_(argReturnType),
		parameterTypes_(std::move(argParameterTypes)) { }
		
		FunctionTypeData FunctionTypeData::copy() const {
			return FunctionTypeData(attributes().copy(),
			                        returnType(),
			                        parameterTypes().copy());
		}
		
		const AST::Context& FunctionTypeData::context() const {
			return returnType()->context();
		}
		
		const FunctionAttributes& FunctionTypeData::attributes() const {
			return attributes_;
		}
		
		const AST::Type* FunctionTypeData::returnType() const {
			return returnType_;
		}
		
		const AST::TypeArray& FunctionTypeData::parameterTypes() const {
			return parameterTypes_;
		}
		
		std::string FunctionTypeData::toString() const {
			return makeString("FunctionType("
			                  "attributes: %s, "
			                  "returnType: %s, "
			                  "parameterTypes: %s)",
			                  attributes().toString().c_str(),
			                  returnType()->toString().c_str(),
			                  makeArrayPtrString(parameterTypes()).c_str());
		}
		
		std::string FunctionTypeData::nameToString() const {
			return makeString("FunctionType("
			                  "attributes: %s, "
			                  "returnType: %s, "
			                  "parameterTypes: %s)",
			                  attributes().toString().c_str(),
			                  returnType()->nameToString().c_str(),
			                  makeNameArrayString(parameterTypes()).c_str());
		}
		
		std::size_t FunctionTypeData::hash() const {
			Hasher hasher;
			hasher.add(attributes());
			hasher.add(returnType());
			hasher.add(parameterTypes());
			return hasher.get();
		}
		
		bool FunctionTypeData::operator==(const FunctionTypeData& other) const {
			return attributes() == other.attributes() &&
			       returnType() == other.returnType() &&
			       parameterTypes() == other.parameterTypes();
		}
		
		FunctionType::FunctionType(FunctionAttributes argAttributes,
		                           const AST::Type* const argReturnType,
		                           AST::TypeArray argParameterTypes)
		: data_(nullptr) {
			FunctionTypeData functionTypeData(std::move(argAttributes),
			                                  argReturnType,
			                                  std::move(argParameterTypes));
			*this = argReturnType->context().getFunctionType(std::move(functionTypeData));
		}
		
		FunctionType FunctionType::substitute(const TemplateVarMap& templateVarMap) const {
			if (templateVarMap.empty()) {
				return *this;
			}
			
			const auto substitutedReturnType = returnType()->substitute(templateVarMap);
			
			bool changed = (substitutedReturnType != returnType());
			
			AST::TypeArray substitutedParameterTypes;
			
			for (const auto parameterType: parameterTypes()) {
				const auto substitutedParameterType = parameterType->substitute(templateVarMap);
				changed |= (substitutedParameterType != parameterType);
				substitutedParameterTypes.push_back(substitutedParameterType);
			}
			
			auto noExceptPredicate = attributes().noExceptPredicate().substitute(templateVarMap);
			changed |= (noExceptPredicate != attributes().noExceptPredicate());
			
			if (changed) {
				FunctionAttributes newAttributes(attributes().isVarArg(),
				                                 attributes().isMethod(),
				                                 attributes().isTemplated(),
				                                 std::move(noExceptPredicate));
				return FunctionType(std::move(newAttributes), substitutedReturnType, std::move(substitutedParameterTypes));
			} else {
				return *this;
			}
		}
		
		FunctionType FunctionType::makeTemplated() const {
			if (attributes().isTemplated()) {
				return *this;
			}
			
			const bool newIsTemplated = true;
			FunctionAttributes newAttributes(attributes().isVarArg(),
			                                 attributes().isMethod(),
			                                 newIsTemplated,
			                                 attributes().noExceptPredicate().copy());
			return FunctionType(std::move(newAttributes), returnType(), parameterTypes().copy());
		}
		
		bool FunctionType::dependsOnAny(const TemplateVarArray& array) const {
			if (returnType()->dependsOnAny(array)) {
				return true;
			}
			
			if (attributes().noExceptPredicate().dependsOnAny(array)) {
				return true;
			}
			
			for (const auto& paramType: parameterTypes()) {
				if (paramType->dependsOnAny(array)) {
					return true;
				}
			}
			
			return false;
		}
		
		bool FunctionType::dependsOnOnly(const TemplateVarArray& array) const {
			if (!returnType()->dependsOnOnly(array)) {
				return false;
			}
			
			if (!attributes().noExceptPredicate().dependsOnOnly(array)) {
				return false;
			}
			
			for (const auto& paramType: parameterTypes()) {
				if (!paramType->dependsOnOnly(array)) {
					return false;
				}
			}
			
			return true;
		}
		
		std::size_t FunctionType::hash() const {
			Hasher hasher;
			hasher.add(data_);
			return hasher.get();
		}
		
	}
	
}
