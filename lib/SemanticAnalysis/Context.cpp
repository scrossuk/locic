#include <boost/functional/hash.hpp>

#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Context::Context(Debug::Module& pDebugModule, SEM::Context& pSemContext)
			: debugModule_(pDebugModule), semContext_(pSemContext),
			templateRequirementsComplete_(false) {
				validVarArgTypes_.insert("byte_t");
				validVarArgTypes_.insert("ubyte_t");
				validVarArgTypes_.insert("short_t");
				validVarArgTypes_.insert("ushort_t");
				validVarArgTypes_.insert("int_t");
				validVarArgTypes_.insert("uint_t");
				validVarArgTypes_.insert("long_t");
				validVarArgTypes_.insert("ulong_t");
				validVarArgTypes_.insert("longlong_t");
				validVarArgTypes_.insert("ulonglong_t");
				validVarArgTypes_.insert("int8_t");
				validVarArgTypes_.insert("uint8_t");
				validVarArgTypes_.insert("int16_t");
				validVarArgTypes_.insert("uint16_t");
				validVarArgTypes_.insert("int32_t");
				validVarArgTypes_.insert("uint32_t");
				validVarArgTypes_.insert("int64_t");
				validVarArgTypes_.insert("uint64_t");
				validVarArgTypes_.insert("float_t");
				validVarArgTypes_.insert("double_t");
				validVarArgTypes_.insert("longdouble_t");
				validVarArgTypes_.insert("__ptr");
			}
		
		Debug::Module& Context::debugModule() {
			return debugModule_;
		}
		
		ScopeStack& Context::scopeStack() {
			return scopeStack_;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return scopeStack_;
		}
		
		SEM::Context& Context::semContext() {
			return semContext_;
		}
		
		std::vector<TemplateInstTuple>& Context::templateInstantiations() {
			return templateInstantiations_;
		}
		
		bool Context::templateRequirementsComplete() const {
			return templateRequirementsComplete_;
		}
		
		void Context::setTemplateRequirementsComplete() {
			templateRequirementsComplete_ = true;
		}
		
		const std::set<std::string>& Context::validVarArgTypes() const {
			return validVarArgTypes_;
		}
		
		const MethodSet* Context::getMethodSet(MethodSet methodSet) const {
			const auto result = methodSets_.insert(std::move(methodSet));
			return &(*(result.first));
		}
		
		Optional<bool> Context::getCapability(const SEM::Type* const type, const char* const capability) const {
			const auto iterator = capabilities_.find(std::make_pair(type, capability));
			if (iterator != capabilities_.end()) {
				return make_optional(iterator->second);
			} else {
				return Optional<bool>();
			}
		}
		
		void Context::setCapability(const SEM::Type* type, const char* capability, const bool isCapable) {
			(void) capabilities_.insert(std::make_pair(std::make_pair(type, capability), isCapable));
		}
		
		std::size_t Context::hashPair::operator()(const std::pair<const SEM::Type*, const char*>& pair) const {
			std::size_t seed = 0;
			boost::hash_combine(seed, pair.first);
			boost::hash_combine(seed, pair.second);
			return seed;
		}
		
		SEM::Value getSelfValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'self' in non-method at %s.", location.toString().c_str()));
			}
			
			if (thisFunction->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access 'self' in static method at %s.",
					location.toString().c_str()));
			}
			
			const auto selfType = thisTypeInstance->selfType();
			
			// TODO: we need to actually put the predicate in the type that's returned!
			const auto selfIsConst = evaluatePredicate(context, thisFunction->constPredicate(), SEM::TemplateVarMap());
			
			const auto selfConstType = selfIsConst ? selfType->createConstType() : selfType;
			return createSelfRef(context, selfConstType);
		}
		
		SEM::Value getThisValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'this' in non-method at %s.",
					location.toString().c_str()));
			}
			
			if (thisFunction->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access 'this' in static method at %s.",
					location.toString().c_str()));
			}
			
			const auto selfType = thisTypeInstance->selfType();
			
			// TODO: we need to actually put the predicate in the type that's returned!
			const auto selfIsConst = evaluatePredicate(context, thisFunction->constPredicate(), SEM::TemplateVarMap());
			
			const auto selfConstType = selfIsConst ? selfType->createConstType() : selfType;
			return SEM::Value::This(getBuiltInType(context.scopeStack(), "__ptr", { selfConstType }));
		}
		
	}
	
}

