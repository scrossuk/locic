#ifndef LOCIC_CODEGEN_CONSTANTGENERATOR_HPP
#define LOCIC_CODEGEN_CONSTANTGENERATOR_HPP

#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		class ConstantGenerator {
			public:
				inline ConstantGenerator(const Module& module)
					: module_(module) { }
					
				inline llvm::Value* getSize(size_t sizeValue) const {
					const size_t sizeTypeWidth = module.getTargetInfo().getPrimitiveSize("size_t");
					return llvm::ConstantInt::get(module.getLLVMContext()
						llvm::APInt(sizeTypeWidth, sizeValue));
				}
				
			private:
				const Module& module_;
				
		}
		
	}
	
}

#endif
