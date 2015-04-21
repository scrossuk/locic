#include <vector>

#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		class UnionSizer {
		public:
			UnionSizer(Module& module)
			: dataLayout_(module.getLLVMModulePtr()),
			maxType_(TypeGenerator(module).getI8Type()),
			maxAlign_(1),
			maxSize_(1) { }
			
			llvm::Type* maxType() const {
				return maxType_;
			}
			
			size_t maxAlign() const {
				return maxAlign_;
			}
			
			size_t maxSize() const {
				return maxSize_;
			}
			
			void addType(llvm::Type* const type) {
				assert(type != nullptr);
				const auto typeSize = dataLayout_.getTypeAllocSize(type);
				const auto typeAlign = dataLayout_.getABITypeAlignment(type);
				
				if (typeAlign > maxAlign_ || (typeAlign == maxAlign_ && typeSize > maxSize_)) {
					maxType_ = type;
					maxAlign_ = typeAlign;
					maxSize_ = typeSize;
				} else {
					assert(typeAlign <= maxAlign_ && typeSize <= maxSize_);
				}
			}
			
		private:
			llvm::DataLayout dataLayout_;
			llvm::Type* maxType_;
			size_t maxAlign_;
			size_t maxSize_;
			
		};
		
		llvm::StructType* genTypeInstance(Module& module, const SEM::TypeInstance* typeInstance) {
			assert(!typeInstance->isInterface());
			
			const auto iterator = module.typeInstanceMap().find(typeInstance);
			if (iterator != module.typeInstanceMap().end()) {
				return iterator->second;
			}
			
			const auto mangledName = mangleObjectType(module, typeInstance);
			const auto structType = TypeGenerator(module).getForwardDeclaredStructType(mangledName);
			
			module.typeInstanceMap().insert(std::make_pair(typeInstance, structType));
			
			// If the size isn't known then just return an opaque struct.
			if (!isObjectTypeSizeKnownInThisModule(module, typeInstance)) {
				size_t index = 0;
				
				// Create mapping between member variables and their
				// indexes within their parent.
				for (const auto& var: typeInstance->variables()) {
					const auto result = module.getMemberVarMap().insert(std::make_pair(var, index++));
					assert(result.second);
					(void) result;
				}
				
				return structType;
			}
			
			if (typeInstance->isUnion()) {
				UnionSizer sizer(module);
				
				assert(!typeInstance->variables().empty());
				
				for (const auto& var: typeInstance->variables()) {
					// All variables at 'index' 0.
					const auto result = module.getMemberVarMap().insert(std::make_pair(var, 0));
					assert(result.second);
					(void) result;
					
					sizer.addType(genType(module, var->type()));
				}
				
				llvm::SmallVector<llvm::Type*, 1> structMembers;
				structMembers.push_back(sizer.maxType());
				structType->setBody(structMembers);
				return structType;
			} else if (typeInstance->isUnionDatatype()) {
				UnionSizer sizer(module);
				
				assert(!typeInstance->variants().empty());
				
				for (auto variantTypeInstance: typeInstance->variants()) {
					sizer.addType(genTypeInstance(module, variantTypeInstance));
				}
				
				llvm::SmallVector<llvm::Type*, 2> structMembers;
				structMembers.push_back(TypeGenerator(module).getI8Type());
				structMembers.push_back(sizer.maxType());
				structType->setBody(structMembers);
				return structType;
			} else {
				// Generating the type for a class or struct definition, so
				// the size and contents of the type instance is known and
				// hence the contents can be specified.
				llvm::SmallVector<llvm::Type*, 10> structMembers;
				
				size_t index = 0;
				
				for (const auto& var: typeInstance->variables()) {
					const auto result = module.getMemberVarMap().insert(std::make_pair(var, index++));
					assert(result.second);
					(void) result;
					
					structMembers.push_back(genType(module, var->type()));
				}
				
				const auto livenessIndicator = getLivenessIndicator(module, *typeInstance);
				if (livenessIndicator.isSuffixByte()) {
					// Inserting the liveness indicator at the end.
					structMembers.push_back(TypeGenerator(module).getI8Type());
				}
				
				if (structMembers.empty()) {
					// All datatypes must be at least one byte in size.
					structMembers.push_back(TypeGenerator(module).getI8Type());
				}
				
				structType->setBody(structMembers);
			}
			
			return structType;
		}
		
	}
	
}

