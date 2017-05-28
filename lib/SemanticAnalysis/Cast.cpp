#include <stdio.h>

#include <stdexcept>

#include <locic/AST/Type.hpp>
#include <locic/Debug.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/CastGenerator.hpp>
#include <locic/SemanticAnalysis/CastOperation.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/SatisfyChecker.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>
#include <locic/SemanticAnalysis/Unifier.hpp>

namespace locic {

	namespace SemanticAnalysis {

		const AST::Type*
		ImplicitCastTypeFormatOnly(Context& context, const AST::Type* sourceType, const AST::Type* destType,
		                           const Debug::SourceLocation& location) {
			Unifier unifier;
			SatisfyChecker satisfyChecker(context, unifier);
			CastGenerator castGenerator(context, satisfyChecker);
			
			auto result = castGenerator.implicitCastNoop(sourceType->resolveAliases(),
			                                             destType->resolveAliases());
			if (result.failed()) return nullptr;
			
			return result.value().type();
		}

		Optional<AST::Value>
		ImplicitCastFormatOnly(Context& context, AST::Value value, const AST::Type* destType,
		                       const Debug::SourceLocation& location) {
			auto resultType = ImplicitCastTypeFormatOnly(context, value.type(), destType, location);
			if (resultType == nullptr) {
				return Optional<AST::Value>();
			}
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (value.type() != resultType) {
				return make_optional(AST::Value::Cast(resultType, std::move(value)));
			} else {
				return make_optional(std::move(value));
			}
		}

		Optional<AST::Value>
		ImplicitCastConvert(Context& context, std::vector<std::string>& errors, AST::Value value,
		                    const AST::Type* destType, const Debug::SourceLocation& location, bool allowBind,
		                    bool formatOnly = false);

		static Optional<AST::Value> PolyCastRefValueToType(Context& context, AST::Value value, const AST::Type* destType) {
			const auto sourceType = value.type();
			assert(sourceType->isRef() && destType->isRef());
			
			const auto sourceTargetType = sourceType->refTarget();
			const auto destTargetType = destType->refTarget();
			
			Unifier unifier;
			const auto result = SatisfyChecker(context, unifier).satisfies(sourceTargetType,
			                                                               destTargetType);
			return result.success() ?
				make_optional(AST::Value::PolyCast(destType, std::move(value))) :
				Optional<AST::Value>();
		}
		
		static Optional<AST::Value> PolyCastTypenameValueToType(Context& context, AST::Value value, const AST::Type* destType) {
			const auto sourceType = value.type();
			assert(sourceType->isTypename() && destType->isTypename());
			
			const auto sourceTargetType = sourceType->typenameTarget();
			const auto destTargetType = destType->typenameTarget();
			
			Unifier unifier;
			const auto result = SatisfyChecker(context, unifier).satisfies(sourceTargetType,
			                                                               destTargetType);
			return result.success() ?
				make_optional(AST::Value::PolyCast(destType, std::move(value))) :
				Optional<AST::Value>();
		}
		
		// User-defined casts.
		static Optional<AST::Value> ImplicitCastUser(Context& context, std::vector<std::string>& errors,
		                                             AST::Value rawValue, const AST::Type* destType,
		                                             const Debug::SourceLocation& location, bool allowBind) {
			auto value = derefValue(std::move(rawValue));
			const auto sourceDerefType = getDerefType(value.type());
			const auto destDerefType = getDerefType(destType)->stripConst();
			
			if (sourceDerefType->isObject() && destDerefType->isObjectOrTemplateVar() &&
			    TypeCapabilities(context).supportsImplicitCast(sourceDerefType)) {
				if (destDerefType->isObject() && sourceDerefType->getObjectType() == destDerefType->getObjectType()) {
					// Can't cast to same type.
					return Optional<AST::Value>();
				}
				
				const auto& castFunction = sourceDerefType->getObjectType()->getFunction(context.getCString("implicitcast"));
				
				const auto& requiresPredicate = castFunction.requiresPredicate();
				
				auto combinedTemplateVarMap = sourceDerefType->generateTemplateVarMap();
				const auto& castTemplateVar = castFunction.templateVariables().front();
				combinedTemplateVarMap.insert(std::make_pair(castTemplateVar, AST::Value::TypeRef(destDerefType, castTemplateVar->type())));
				
				if (evaluatePredicate(context, requiresPredicate, combinedTemplateVarMap).success()) {
					auto boundValue = bindReference(context, std::move(value));
					auto method = GetTemplatedMethod(context, std::move(boundValue), context.getCString("implicitcast"), makeTemplateArgs(context, { destDerefType }), location);
					auto castValue = CallValue(context, std::move(method), {}, location);
					
					// There still might be some aspects to cast with the constructed type.
					return ImplicitCastConvert(context, errors, std::move(castValue), destType, location, allowBind);
				} else {
					errors.push_back(makeString("user cast failed from type '%s' to type '%s'",
					                            sourceDerefType->toString().c_str(),
					                            destDerefType->toString().c_str()));
				}
			}
			
			return Optional<AST::Value>();
		}
		
		static bool isStructurallyEqual(const AST::Type* firstType, const AST::Type* secondType) {
			if (firstType->kind() != secondType->kind()) {
				return false;
			}
			
			if (firstType->isObject()) {
				return firstType->getObjectType() == secondType->getObjectType();
			} else if (firstType->isTemplateVar()) {
				return firstType->getTemplateVar() == secondType->getTemplateVar();
			} else {
				return false;
			}
		}
		
		static bool canTreatConstantAsUnsigned(const AST::Value& value, const AST::Type* const destType) {
			assert(value.isConstant());
			
			const auto sourceType = value.type()->resolveAliases();
			
			if (!sourceType->isPrimitive() || !destType->isPrimitive()) {
				return false;
			}
			
			const auto sourcePrimitiveID = sourceType->primitiveID();
			const auto destPrimitiveID = destType->primitiveID();
			
			if (!sourcePrimitiveID.isSignedInteger()) {
				// Looking for constants of signed integer type.
				return false;
			}
			
			if (!destPrimitiveID.isUnsignedInteger()) {
				// Looking to cast to an unsigned integer type.
				return false;
			}
			
			// Finally, we need to able to represent the constant
			// in the destination type.
			return sourcePrimitiveID.asUnsigned().isSubsetOf(destPrimitiveID);
		}
		
		Diag FormatOnlyCastFailedDiag(const AST::Type* const sourceType,
		                              const AST::Type* const destType) {
			return Error("Format only cast failed from type %s to type %s.",
			             sourceType->toDiagString().c_str(), destType->toDiagString().c_str());
		}

		Optional<AST::Value> ImplicitCastConvert(Context& context, std::vector<std::string>& errors, const AST::Value value, const AST::Type* destType, const Debug::SourceLocation& location, bool allowBind, bool formatOnly) {
			{
				// Try a format only cast first, since
				// this requires no transformations.
				auto castResult = ImplicitCastFormatOnly(context, value.copy(), destType, location);
				if (castResult) {
					return castResult;
				} else if (formatOnly) {
					context.issueDiag(FormatOnlyCastFailedDiag(value.type(), destType), location);
					throw SkipException();
				}
			}
			
			const auto sourceType = value.type()->resolveAliases();
			
			if (value.isConstant() &&
			    canTreatConstantAsUnsigned(value, destType)) {
				// Allow positive signed integer constants to be
				// treated as unsigned.
				return make_optional(AST::Value::Constant(value.constant(),
				                                          destType));
			}
			
			// Try to cast datatype to its parent variant.
			if (sourceType->isDatatype()) {
				const auto destDerefType = getDerefType(destType);
				if (destDerefType->isVariant()) {
					bool found = false;
					for (const auto variantType: destDerefType->getObjectType()->variantTypes()) {
						if (sourceType->getObjectType() == variantType->getObjectType()) {
							found = true;
							break;
						}
					}
					
					if (found) {
						auto castValue = AST::Value::Cast(destDerefType->stripConst(), value.copy());
						auto castResult = ImplicitCastConvert(context, errors, std::move(castValue), destType, location, allowBind);
						if (castResult) {
							return castResult;
						}
					}
				}
			}
			
			// Try to dereference the source ref type enough times
			// so that it matches the destination ref type.
			{
				const auto sourceCount = getRefCount(sourceType);
				const auto destCount = getRefCount(destType);
				
				// Can only ever reduce a reference to another
				// reference count without implicitCopy.
				if (sourceCount > destCount && destCount > 0) {
					auto reducedValue = value.copy();
					
					const auto numReduce = sourceCount - destCount;
					for (size_t i = 0; i < numReduce; i++) {
						reducedValue = derefOne(std::move(reducedValue));
					}
					
					auto castResult = ImplicitCastConvert(context, errors, std::move(reducedValue), destType, location, allowBind);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to use a polymorphic ref cast.
			if (sourceType->isRef() && destType->isRef() && destType->refTarget()->isInterface()) {
				const auto sourceTarget = sourceType->refTarget();
				const auto destTarget = destType->refTarget();
				
				if (sourceTarget->constPredicate().implies(destTarget->constPredicate())) {
					auto castResult = PolyCastRefValueToType(context, value.copy(), destType);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to use a polymorphic typename cast.
			if (sourceType->isTypename() && destType->isTypename() && sourceType->typenameTarget()->isObject() && destType->typenameTarget()->isInterface()) {
				const auto sourceTarget = sourceType->typenameTarget();
				const auto destTarget = destType->typenameTarget();
				
				if (sourceTarget->constPredicate().implies(destTarget->constPredicate())) {
					auto castResult = PolyCastTypenameValueToType(context, value.copy(), destType);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to use implicitCopy-by-reference to turn a
			// reference into a basic value.
			if (sourceType->isRef() && (!destType->isRef() || !isStructurallyEqual(sourceType->refTarget(), destType->refTarget()))) {
				const auto sourceDerefType = getDerefType(sourceType);
				if (TypeCapabilities(context).supportsImplicitCopy(sourceDerefType)) {
					auto copyValue = CallValue(context, GetSpecialMethod(context, derefValue(value.copy()), context.getCString("implicitcopy"), location), {}, location);
					
					const bool nextAllowBind = false;
					auto convertCast = ImplicitCastConvert(context, errors, std::move(copyValue), destType, location, nextAllowBind);
					if (convertCast) {
						return convertCast;
					}
				} else if (sourceDerefType->isObjectOrTemplateVar() && CanDoImplicitCast(context, sourceDerefType->stripConst(), destType, location)) {
					// This almost certainly would have worked
					// if implicitCopy was available, so let's
					// report this error to the user.
					errors.push_back(makeString("unable to copy type '%s' because it doesn't have "
					                            "a valid 'implicitcopy' method, "
					                            "in cast from type '%s' to type '%s'",
					                            sourceDerefType->toDiagString().c_str(),
					                            sourceType->toDiagString().c_str(),
					                            destType->toDiagString().c_str()));
				}
			}
			
			// Try to use implicitCopy to make a value non-const.
			if (getRefCount(sourceType) == getRefCount(destType) &&
			    !sourceType->constPredicate().implies(destType->constPredicate()) &&
			    sourceType->isObjectOrTemplateVar() && TypeCapabilities(context).supportsImplicitCopy(sourceType)) {
				auto boundValue = bindReference(context, value.copy());
				auto copyValue = CallValue(context, GetSpecialMethod(context, std::move(boundValue), context.getCString("implicitcopy"), location), {}, location);
				assert(copyValue.type()->constPredicate().implies(destType->constPredicate()));
				
				const bool nextAllowBind = false;
				auto convertCast = ImplicitCastConvert(context, errors, std::move(copyValue), destType, location, nextAllowBind);
				if (convertCast) {
					return convertCast;
				}
			}
			
			// Try to bind value to reference (e.g. T -> T&).
			if (allowBind && !sourceType->isRef() && destType->isRef() &&
			    sourceType->constPredicate().implies(destType->refTarget()->constPredicate()) &&
			    isStructurallyEqual(sourceType, destType->refTarget())) {
				auto refValue = bindReference(context, value.copy());
				auto castResult = ImplicitCastConvert(context, errors, std::move(refValue), destType, location, allowBind);
				if (castResult) {
					return castResult;
				}
			}
			
			// Try a user cast.
			{
				auto castResult = ImplicitCastUser(context, errors, value.copy(), destType, location, allowBind);
				if (castResult) {
					return castResult;
				}
			}
			
			return Optional<AST::Value>();
		}

		Diag
		CannotImplicitlyCastTypeDiag(const AST::Type* const sourceType,
		                             const AST::Type* const destType) {
			return Error("Can't implicitly cast type '%s' to type '%s'.",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}

		Diag
		CannotImplicitlyCastValueToTypeDiag(const AST::Type* const sourceType,
		                                    const AST::Type* const destType) {
			return Error("Can't implicitly cast value of type '%s' to type '%s'.",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}

		Diag
		CastErrorDiag(std::string message) {
			return Error(message.c_str());
		}

		AST::Value ImplicitCast(Context& context, AST::Value value, const AST::Type* destType, const Debug::SourceLocation& location, bool formatOnly) {
			assert(value.type()->canBeUsedAsValue());
			assert(destType->canBeUsedAsValue());
			
			std::vector<std::string> errors;
			const auto valueKind = value.kind();
			const auto valueType = value.type();
			const bool allowBind = true;
			auto result = ImplicitCastConvert(context, errors, std::move(value), destType->resolveAliases(), location, allowBind, formatOnly);
			if (result) {
				return std::move(*result);
			}
			
			if (errors.empty()) {
				if (valueKind == AST::Value::CASTDUMMYOBJECT) {
					context.issueDiag(CannotImplicitlyCastTypeDiag(valueType, destType), location);
				} else {
					context.issueDiag(CannotImplicitlyCastValueToTypeDiag(valueType, destType),
					                  location);
				}
			} else {
				context.issueDiag(CastErrorDiag(errors.front()), location);
			}
			
			return AST::Value::CastDummy(destType);
		}
		
		bool CanDoImplicitCast(Context& context, const AST::Type* sourceType, const AST::Type* destType, const Debug::SourceLocation& location) {
			assert(sourceType->canBeUsedAsValue());
			assert(destType->canBeUsedAsValue());
			
			const bool allowBind = true;
			const bool formatOnly = false;
			std::vector<std::string> errors;
			const auto result = ImplicitCastConvert(context, errors, AST::Value::CastDummy(sourceType), destType, location, allowBind, formatOnly);
			return result;
		}
		
		namespace {
			
			const AST::Type* getVariantParent(const AST::Type* type) {
				while (type->isRef()) {
					type = type->refTarget();
				}
				
				if (!type->isObject()) {
					return nullptr;
				}
				
				if (type->getObjectType()->parentTypeInstance() == nullptr) {
					return nullptr;
				}
				
				return type->getObjectType()->parentTypeInstance()->selfType();
			}
			
		}
		
		const AST::Type* UnifyTypes(Context& context, const AST::Type* first, const AST::Type* second, const Debug::SourceLocation& location) {
			// Try to convert both types to their parent (if any).
			const auto firstParent = getVariantParent(first);
			if (firstParent != nullptr &&
				CanDoImplicitCast(context, first, firstParent, location) &&
				CanDoImplicitCast(context, second, firstParent, location)) {
				return firstParent;
			}
			
			if (CanDoImplicitCast(context, first, second, location)) {
				return second;
			} else {
				return first;
			}
		}
		
	}
	
}





