#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <stdint.h>

#include <locic/SEM.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		class ArgInfo {
			public:
				inline static ArgInfo None() {
					return ArgInfo(false, false, 0);
				}
				
				inline static ArgInfo ContextOnly() {
					return ArgInfo(false, true, 0);
				}
				
				inline ArgInfo(bool hRVA, bool hCA,
							   size_t nSA)
					: hasReturnVarArgument_(hRVA),
					  hasContextArgument_(hCA),
					  numStandardArguments_(nSA) { }
					  
				bool hasReturnVarArgument() const {
					return hasReturnVarArgument_;
				}
				
				bool hasContextArgument() const {
					return hasContextArgument_;
				}
				
				size_t contextArgumentOffset() const {
					return hasReturnVarArgument() ? 1 : 0;
				}
				
				size_t standardArgumentOffset() const {
					return contextArgumentOffset() +
						   (hasContextArgument() ? 1 : 0);
				}
				
				size_t numStandardArguments() const {
					return numStandardArguments_;
				}
				
				size_t numArguments() const {
					return standardArgumentOffset() +
						   numStandardArguments();
				}
				
			private:
				bool hasReturnVarArgument_;
				bool hasContextArgument_;
				size_t numStandardArguments_;
				
		};
		
		inline ArgInfo getArgInfo(Module& module, SEM::Function* function) {
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, function->type()->getFunctionReturnType());
			const bool hasContextArg = function->isMethod() && !function->isStaticMethod();
			return ArgInfo(hasReturnVarArg, hasContextArg,
						   function->type()->getFunctionParameterTypes().size());
		}
		
	}
	
}

#endif
