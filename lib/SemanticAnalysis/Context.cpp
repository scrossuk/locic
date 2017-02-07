#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <locic/AST/Type.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Debug.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/SharedMaps.hpp>
#include <locic/Support/StableSet.hpp>
#include <locic/Support/StringHost.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TemplateInst.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class ContextImpl {
		public:
			ContextImpl(Context& context, const SharedMaps& argSharedMaps,
			            Debug::Module& pDebugModule, AST::Context& pSemContext,
			            DiagnosticReceiver& argDiagReceiver)
			: aliasTypeResolver(context),
			sharedMaps(argSharedMaps),
			stringHost(sharedMaps.stringHost()),
			debugModule(pDebugModule),
			astContext(pSemContext),
			diagReceiver(&argDiagReceiver),
			typeBuilder(context),
			methodSetsComplete(false),
			templateRequirementsComplete(false) {
			}
			
			template <typename Key, typename Value>
			struct hashPair {
				std::size_t operator()(const std::pair<Key, Value>& pair) const {
					std::size_t seed = 0;
					std::hash<Key> keyHashFn;
					boost::hash_combine(seed, keyHashFn(pair.first));
					std::hash<Value> valueHashFn;
					boost::hash_combine(seed, valueHashFn(pair.second));
					return seed;
				}
			};
			
			AliasTypeResolver aliasTypeResolver;
			const SharedMaps& sharedMaps;
			const StringHost& stringHost;
			Debug::Module& debugModule;
			ScopeStack scopeStack;
			AST::Context& astContext;
			DiagnosticReceiver* diagReceiver;
			TypeBuilder typeBuilder;
			bool methodSetsComplete;
			bool templateRequirementsComplete;
			std::vector<TemplateInst> templateInstantiations;
			std::unordered_map<std::pair<const AST::Type*, String>, bool, hashPair<const AST::Type*, String>> capabilities;
			
			std::unordered_map<const AST::Type*, const AST::MethodSet*> objectMethodSetMap;
			std::unordered_map<std::pair<const AST::TemplatedObject*, const AST::Type*>, const AST::MethodSet*,
				hashPair<const AST::TemplatedObject*, const AST::Type*>> templateVarMethodSetMap;
			
			std::vector<std::pair<const AST::Type*, const AST::Type*>> assumedSatisfyPairs;
			std::vector<std::pair<const AST::TemplateVar*, const AST::Predicate*>> computingMethodSetTemplateVars;
		};
		
		Context::Context(const SharedMaps& argSharedMaps, Debug::Module& argDebugModule,
		                 AST::Context& argSemContext, DiagnosticReceiver& diagReceiver)
		: impl_(new ContextImpl(*this, argSharedMaps, argDebugModule, argSemContext,
		                        diagReceiver)) { }
		
		Context::~Context() { }
		
		DiagnosticReceiver& Context::diagnosticReceiver() {
			return *(impl_->diagReceiver);
		}
		
		void Context::setDiagnosticReceiver(DiagnosticReceiver& receiver) {
			impl_->diagReceiver = &receiver;
		}
		
		void Context::issueDiagPtr(std::unique_ptr<Diag> diag,
		                           const Debug::SourceLocation& location,
		                           OptionalDiag chain) {
			diagnosticReceiver().issueDiag(std::move(diag), location, std::move(chain));
		}
		
		AliasTypeResolver& Context::aliasTypeResolver() {
			return impl_->aliasTypeResolver;
		}
		
		const SharedMaps& Context::sharedMaps() const {
			return impl_->sharedMaps;
		}
		
		String Context::getCString(const char* const cString) const {
			return String(impl_->stringHost, cString);
		}
		
		String Context::getString(const std::string& string) const {
			return String(impl_->stringHost, string);
		}
		
		Debug::Module& Context::debugModule() {
			return impl_->debugModule;
		}
		
		ScopeStack& Context::scopeStack() {
			return impl_->scopeStack;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return impl_->scopeStack;
		}
		
		AST::Context& Context::astContext() {
			return impl_->astContext;
		}
		
		TypeBuilder& Context::typeBuilder() {
			return impl_->typeBuilder;
		}
		
		const AST::MethodSet*
		Context::findMethodSet(const AST::TemplatedObject* const templatedObject,
		                       const AST::Type* const type) const {
			assert(methodSetsComplete());
			assert(type->isObject() || type->isTemplateVar());
			
			if (type->isObject()) {
				const auto iterator = impl_->objectMethodSetMap.find(type);
				return iterator != impl_->objectMethodSetMap.end() ? iterator->second : nullptr;
			} else {
				assert(templatedObject != nullptr);
				const auto iterator = impl_->templateVarMethodSetMap.find(std::make_pair(templatedObject, type));
				return iterator != impl_->templateVarMethodSetMap.end() ? iterator->second : nullptr;
			}
		}
		
		void Context::addMethodSet(const AST::TemplatedObject* const templatedObject,
		                           const AST::Type* const type,
		                           const AST::MethodSet* const methodSet) {
			assert(methodSetsComplete());
			assert(type->isObject() || type->isTemplateVar());
			
			if (type->isObject()) {
				const auto result = impl_->objectMethodSetMap.insert(std::make_pair(type, methodSet));
				if (!result.second) {
					// Overwrite any existing element.
					result.first->second = methodSet;
				}
			} else {
				assert(templatedObject != nullptr);
				const auto result = impl_->templateVarMethodSetMap.insert(std::make_pair(std::make_pair(templatedObject, type), methodSet));
				if (!result.second) {
					// Overwrite any existing element.
					result.first->second = methodSet;
				}
			}
		}
		
		std::vector<TemplateInst>& Context::templateInstantiations() {
			return impl_->templateInstantiations;
		}
		
		bool Context::templateRequirementsComplete() const {
			return impl_->templateRequirementsComplete;
		}
		
		void Context::setTemplateRequirementsComplete() {
			assert(!impl_->templateRequirementsComplete);
			impl_->templateRequirementsComplete = true;
		}
		
		bool Context::methodSetsComplete() const {
			return impl_->methodSetsComplete;
		}
		
		void Context::setMethodSetsComplete() {
			assert(!impl_->methodSetsComplete);
			impl_->methodSetsComplete = true;
		}
		
		Optional<bool> Context::getCapability(const AST::Type* const type, const String& capability) const {
			const auto iterator = impl_->capabilities.find(std::make_pair(type, capability));
			if (iterator != impl_->capabilities.end()) {
				return make_optional(iterator->second);
			} else {
				return Optional<bool>();
			}
		}
		
		void Context::setCapability(const AST::Type* type, const String& capability, const bool isCapable) {
			(void) impl_->capabilities.insert(std::make_pair(std::make_pair(type, capability), isCapable));
		}
		
		bool Context::isComputingMethodSet(const AST::TemplateVar* const templateVar, const AST::Predicate& predicate) const {
			for (const auto& computingMethodSetPair: impl_->computingMethodSetTemplateVars) {
				if (templateVar == computingMethodSetPair.first && predicate == *(computingMethodSetPair.second)) {
					return true;
				}
			}
			return false;
		}
		
		void Context::pushComputingMethodSet(const AST::TemplateVar* const  templateVar, const AST::Predicate& predicate) {
			impl_->computingMethodSetTemplateVars.push_back(std::make_pair(templateVar, &predicate));
		}
		
		void Context::popComputingMethodSet() {
			impl_->computingMethodSetTemplateVars.pop_back();
		}
		
		bool Context::isAssumedSatisfies(const AST::Type* const checkType, const AST::Type* const requireType) const {
			const auto pair = std::make_pair(checkType, requireType);
			for (const auto& assumedSatisfyPair: impl_->assumedSatisfyPairs) {
				if (pair == assumedSatisfyPair) {
					return true;
				}
			}
			return false;
		}
		
		void Context::pushAssumeSatisfies(const AST::Type* const checkType, const AST::Type* const requireType) {
			const auto pair = std::make_pair(checkType, requireType);
			impl_->assumedSatisfyPairs.push_back(pair);
		}
		
		void Context::popAssumeSatisfies() {
			impl_->assumedSatisfyPairs.pop_back();
		}
		
		class SelfInNonMethodDiag: public Error {
		public:
			SelfInNonMethodDiag() { }
			
			std::string toString() const {
				return "cannot access 'self' in non-method function";
			}
			
		};
		
		class SelfInStaticMethodDiag: public Error {
		public:
			SelfInStaticMethodDiag() { }
			
			std::string toString() const {
				return "cannot access 'self' in static method";
			}
			
		};
		
		AST::Value getSelfValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				context.issueDiag(SelfInNonMethodDiag(), location);
			}
			
			if (thisFunction->isStaticMethod()) {
				context.issueDiag(SelfInStaticMethodDiag(), location);
			}
			
			const auto selfType = thisTypeInstance != nullptr ? thisTypeInstance->selfType() : context.typeBuilder().getVoidType();
			
			auto predicate = thisFunction->constPredicate().copy();
			predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::SelfConst());
			const auto selfConstType = selfType->createConstType(std::move(predicate));
			return createSelfRef(context, selfConstType);
		}
		
		class ThisInNonMethodDiag: public Error {
		public:
			ThisInNonMethodDiag() { }
			
			std::string toString() const {
				return "cannot access 'this' in non-method function";
			}
			
		};
		
		class ThisInStaticMethodDiag: public Error {
		public:
			ThisInStaticMethodDiag() { }
			
			std::string toString() const {
				return "cannot access 'this' in static method";
			}
			
		};
		
		AST::Value getThisValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				context.issueDiag(ThisInNonMethodDiag(), location);
			}
			
			if (thisFunction->isStaticMethod()) {
				context.issueDiag(ThisInStaticMethodDiag(), location);
			}
			
			const auto selfType = thisTypeInstance != nullptr ? thisTypeInstance->selfType() : context.typeBuilder().getVoidType();
			
			auto predicate = thisFunction->constPredicate().copy();
			predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::SelfConst());
			const auto selfConstType = selfType->createConstType(std::move(predicate));
			return AST::Value::This(getBuiltInType(context, context.getCString("ptr_t"), { selfConstType }));
		}
		
	}
	
}

