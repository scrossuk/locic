#include <assert.h>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		TemplatedObject TemplatedObject::TypeInstance(SEM::TypeInstance* typeInstance) {
			assert(typeInstance != nullptr && !typeInstance->isInterface());
			TemplatedObject object(TYPEINSTANCE);
			object.data_.typeInstance = typeInstance;
			return object;
		}
		
		TemplatedObject TemplatedObject::Function(SEM::TypeInstance* parentTypeInstance, SEM::Function* function) {
			TemplatedObject object(FUNCTION);
			object.data_.functionPair.parentTypeInstance = parentTypeInstance;
			object.data_.functionPair.function = function;
			return object;
		}
		
		TemplatedObject::Kind TemplatedObject::kind() const {
			return kind_;
		}
		
		bool TemplatedObject::isTypeInstance() const {
			return kind_ == TYPEINSTANCE;
		}
		
		bool TemplatedObject::isFunction() const {
			return kind_ == FUNCTION;
		}
		
		SEM::TypeInstance* TemplatedObject::typeInstance() const {
			assert(isTypeInstance());
			return data_.typeInstance;
		}
		
		SEM::TypeInstance* TemplatedObject::parentTypeInstance() const {
			assert(isFunction());
			return data_.functionPair.parentTypeInstance;
		}
		
		SEM::Function* TemplatedObject::function() const {
			assert(isFunction());
			return data_.functionPair.function;
		}
		
		bool TemplatedObject::operator==(const TemplatedObject& other) const {
			if (kind() != other.kind()) {
				return false;
			}
			
			switch (kind()) {
				case TYPEINSTANCE:
					return typeInstance() == other.typeInstance();
				case FUNCTION:
					return parentTypeInstance() == other.parentTypeInstance() &&
						function() == other.function();
				default:
					llvm_unreachable("Unknown templated object kind.");
			}
		}
		
		bool TemplatedObject::operator!=(const TemplatedObject& other) const {
			return !(*this == other);
		}
		
		bool TemplatedObject::operator<(const TemplatedObject& other) const {
			if (kind() != other.kind()) {
				return kind() < other.kind();
			}
			
			switch (kind()) {
				case TYPEINSTANCE:
					return typeInstance() < other.typeInstance();
				case FUNCTION:
					if (parentTypeInstance() != other.parentTypeInstance()) {
						return parentTypeInstance() < other.parentTypeInstance();
					}
					return function() < other.function();
				default:
					llvm_unreachable("Unknown templated object kind.");
			}
		}
				
		TemplatedObject::TemplatedObject(Kind pKind)
			: kind_(pKind) { }
		
		TemplateInst TemplateInst::Type(SEM::Type* type) {
			assert(type->isObject());
			return TemplateInst(TemplatedObject::TypeInstance(type->getObjectType()), type->templateArguments());
		}
		
		TemplateInst TemplateInst::Function(SEM::Type* parentType, SEM::Function* function, llvm::ArrayRef<SEM::Type*> functionArgs) {
			if (parentType != nullptr) {
				assert(parentType->isObject());
				llvm::SmallVector<SEM::Type*, 10> args;
				for (auto arg: parentType->templateArguments()) {
					args.push_back(arg);
				}
				for (size_t i = 0; i < functionArgs.size(); i++) {
					args.push_back(functionArgs[i]);
				}
				return TemplateInst(TemplatedObject::Function(parentType->getObjectType(), function), args);
			} else {
				return TemplateInst(TemplatedObject::Function(nullptr, function), functionArgs);
			}
		}
		
		TemplateInst::TemplateInst(TemplatedObject pObject, llvm::ArrayRef<SEM::Type*> pArguments)
			: object_(pObject) {
				for (size_t i = 0; i < pArguments.size(); i++) {
					arguments_.push_back(pArguments[i]);
				}
			}
		
		TemplatedObject TemplateInst::object() const {
			return object_;
		}
		
		llvm::ArrayRef<SEM::Type*> TemplateInst::arguments() const {
			return arguments_;
		}
		
		bool TemplateInst::operator<(const TemplateInst& other) const {
			if (object() != other.object()) {
				return object() < other.object();
			}
			
			if (arguments().size() != other.arguments().size()) {
				return arguments().size() < other.arguments().size();
			}
			
			for (size_t i = 0; i < arguments().size(); i++) {
				if (arguments()[i] != other.arguments()[i]) {
					return arguments()[i] < other.arguments()[i];
				}
			}
			
			return false;
		}
		
	}
	
}

