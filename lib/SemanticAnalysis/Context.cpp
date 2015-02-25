#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <locic/Array.hpp>
#include <locic/Debug.hpp>
#include <locic/StableSet.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class ContextImpl {
		public:
			ContextImpl(Debug::Module& pDebugModule, SEM::Context& pSemContext)
			: debugModule(pDebugModule), semContext(pSemContext),
			templateRequirementsComplete(false) {
				validVarArgTypes.insert("byte_t");
				validVarArgTypes.insert("ubyte_t");
				validVarArgTypes.insert("short_t");
				validVarArgTypes.insert("ushort_t");
				validVarArgTypes.insert("int_t");
				validVarArgTypes.insert("uint_t");
				validVarArgTypes.insert("long_t");
				validVarArgTypes.insert("ulong_t");
				validVarArgTypes.insert("longlong_t");
				validVarArgTypes.insert("ulonglong_t");
				validVarArgTypes.insert("int8_t");
				validVarArgTypes.insert("uint8_t");
				validVarArgTypes.insert("int16_t");
				validVarArgTypes.insert("uint16_t");
				validVarArgTypes.insert("int32_t");
				validVarArgTypes.insert("uint32_t");
				validVarArgTypes.insert("int64_t");
				validVarArgTypes.insert("uint64_t");
				validVarArgTypes.insert("float_t");
				validVarArgTypes.insert("double_t");
				validVarArgTypes.insert("longdouble_t");
				validVarArgTypes.insert("__ptr");
			}
			
			struct hashPair {
				std::size_t operator()(const std::pair<const SEM::Type*, const char*>& pair) const {
					std::size_t seed = 0;
					boost::hash_combine(seed, pair.first);
					boost::hash_combine(seed, pair.second);
					return seed;
				}
			};
			
			Debug::Module& debugModule;
			ScopeStack scopeStack;
			SEM::Context& semContext;
			bool templateRequirementsComplete;
			std::vector<TemplateInstTuple> templateInstantiations;
			std::set<std::string> validVarArgTypes;
			mutable StableSet<MethodSet> methodSets;
			std::unordered_map<std::pair<const SEM::Type*, const char*>, bool, hashPair> capabilities;
		};
		
		Context::Context(Debug::Module& pDebugModule, SEM::Context& pSemContext)
		: impl_(new ContextImpl(pDebugModule, pSemContext)) { }
		
		Context::~Context() { }
		
		Debug::Module& Context::debugModule() {
			return impl_->debugModule;
		}
		
		ScopeStack& Context::scopeStack() {
			return impl_->scopeStack;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return impl_->scopeStack;
		}
		
		SEM::Context& Context::semContext() {
			return impl_->semContext;
		}
		
		std::vector<TemplateInstTuple>& Context::templateInstantiations() {
			return impl_->templateInstantiations;
		}
		
		bool Context::templateRequirementsComplete() const {
			return impl_->templateRequirementsComplete;
		}
		
		void Context::setTemplateRequirementsComplete() {
			impl_->templateRequirementsComplete = true;
		}
		
		const std::set<std::string>& Context::validVarArgTypes() const {
			return impl_->validVarArgTypes;
		}
		
		const MethodSet* Context::getMethodSet(MethodSet methodSet) const {
			const auto result = impl_->methodSets.insert(std::move(methodSet));
			return &(*(result.first));
		}
		
		Optional<bool> Context::getCapability(const SEM::Type* const type, const char* const capability) const {
			const auto iterator = impl_->capabilities.find(std::make_pair(type, capability));
			if (iterator != impl_->capabilities.end()) {
				return make_optional(iterator->second);
			} else {
				return Optional<bool>();
			}
		}
		
		void Context::setCapability(const SEM::Type* type, const char* capability, const bool isCapable) {
			(void) impl_->capabilities.insert(std::make_pair(std::make_pair(type, capability), isCapable));
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

