#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <memory>
#include <string>
#include <vector>

#include <locic/Debug/VarInfo.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/SEM/TemplateVarMap.hpp>

namespace locic {
	
	namespace SEM {
		
		class TemplateVar;
		class Type;
		
		class Var {
			public:
				static std::unique_ptr<Var> Any(const Type* constructType);
				
				static std::unique_ptr<Var> Basic(const Type* constructType, const Type* type);
				
				static std::unique_ptr<Var> Composite(const Type* type,
				                                      std::vector<std::unique_ptr<Var>> children);
				
				enum Kind {
					ANY,
					BASIC,
					COMPOSITE
				};
				
				Kind kind() const;
				
				bool isAny() const;
				bool isBasic() const;
				bool isComposite() const;
				
				const Type* constructType() const;
				const Type* type() const;
				const std::vector<std::unique_ptr<Var>>& children() const;
				
				bool isUsed() const;
				void setUsed();
				
				bool isMarkedUnused() const;
				void setMarkedUnused(bool isMarkedUnused);
				
				bool isOverrideConst() const;
				void setOverrideConst(bool isOverrideConst);
				
				size_t index() const;
				void setIndex(size_t index);
				
				void setDebugInfo(Debug::VarInfo debugInfo);
				Optional<Debug::VarInfo> debugInfo() const;
				
				std::string toString() const;
				
			private:
				Var();
				
				Kind kind_;
				const Type* constructType_;
				const Type* type_;
				std::vector<std::unique_ptr<Var>> children_;
				bool isUsed_;
				bool isMarkedUnused_;
				bool isOverrideConst_;
				size_t index_;
				Optional<Debug::VarInfo> debugInfo_;
				
		};
		
	}
	
}

#endif
