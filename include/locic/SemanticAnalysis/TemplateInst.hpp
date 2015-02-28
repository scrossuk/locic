#ifndef LOCIC_SEMANTICANALYSIS_TEMPLATEINST_HPP
#define LOCIC_SEMANTICANALYSIS_TEMPLATEINST_HPP

#include <utility>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/Support/Name.hpp>

namespace locic {
	
	namespace SEM {
		
		class TemplatedObject;
		
	}
	
	namespace SemanticAnalysis {
		
		class TemplateInst {
		public:
			TemplateInst(ScopeStack argScopeStack, SEM::TemplateVarMap argTemplateVarMap,
				const SEM::TemplatedObject* argTemplatedObject, Name argName, Debug::SourceLocation argLocation)
			: scopeStack_(std::move(argScopeStack)),
			templateVarMap_(std::move(argTemplateVarMap)),
			templatedObject_(std::move(argTemplatedObject)),
			name_(std::move(argName)),
			location_(std::move(argLocation)) { }
			
			TemplateInst(TemplateInst&&) = default;
			TemplateInst& operator=(TemplateInst&&) = default;
			
			ScopeStack& scopeStack() {
				return scopeStack_;
			}
			
			const SEM::TemplateVarMap& templateVarMap() const {
				return templateVarMap_;
			}
			
			const SEM::TemplatedObject* templatedObject() const {
				return templatedObject_;
			}
			
			const Name& name() const {
				return name_;
			}
			
			const Debug::SourceLocation& location() const {
				return location_;
			}
			
		private:
			TemplateInst(const TemplateInst&) = delete;
			TemplateInst& operator=(const TemplateInst&) = delete;
			
			ScopeStack scopeStack_;
			SEM::TemplateVarMap templateVarMap_;
			const SEM::TemplatedObject* templatedObject_;
			Name name_;
			Debug::SourceLocation location_;
			
		};
		
	}
	
}

#endif
