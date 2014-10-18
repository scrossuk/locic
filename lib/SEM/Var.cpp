#include <assert.h>
#include <stdexcept>
#include <string>

#include <locic/String.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Var* Var::Any(const Type* constructType) {
			Var* var = new Var;
			var->kind_ = ANY;
			var->constructType_ = constructType;
			return var;
		}
		
		Var* Var::Basic(const Type* constructType, const Type* type) {
			Var* var = new Var;
			var->kind_ = BASIC;
			var->constructType_ = constructType;
			var->type_ = type;
			return var;
		}
		
		Var* Var::Composite(const Type* type, const std::vector<Var*>& children) {
			Var* var = new Var;
			var->kind_ = COMPOSITE;
			var->constructType_ = type;
			var->type_ = type;
			var->children_ = children;
			return var;
		}
		
		Var::Var() : kind_(ANY), constructType_(NULL), type_(NULL) { }
		
		Var::Kind Var::kind() const {
			return kind_;
		}
		
		bool Var::isAny() const {
			return kind() == ANY;
		}
		
		bool Var::isBasic() const {
			return kind() == BASIC;
		}
		
		bool Var::isComposite() const {
			return kind() == COMPOSITE;
		}
		
		const Type* Var::constructType() const {
			return constructType_;
		}
		
		const Type* Var::type() const {
			assert(isBasic() || isComposite());
			return type_;
		}
		
		const std::vector<Var*>& Var::children() const {
			assert(isComposite());
			return children_;
		}
		
		Var* Var::substitute(const TemplateVarMap& templateVarMap) const {
			switch (kind()) {
				case ANY:
					return Var::Any(constructType()->substitute(templateVarMap));
				case BASIC:
					return Var::Basic(constructType()->substitute(templateVarMap), type()->substitute(templateVarMap));
				case COMPOSITE:
				{
					std::vector<Var*> substitutedChildren;
					for (const auto var: children()) {
						substitutedChildren.push_back(var->substitute(templateVarMap));
					}
					return Var::Composite(type()->substitute(templateVarMap), substitutedChildren);
				}
				default:
					throw std::runtime_error("Unknown var kind.");
			}
		}
		
		std::string Var::toString() const {
			switch (kind()) {
				case ANY:
					return makeString("Var[ANY](constructType: %s)",
							constructType()->toString().c_str());
				case BASIC:
					return makeString("Var[BASIC](constructType: %s, type: %s)",
							constructType()->toString().c_str(),
							type()->toString().c_str());
				case COMPOSITE:
					return makeString("Var[COMPOSITE](type: %s, children: %s)", type()->toString().c_str(),
							makeArrayString(children()).c_str());
				default:
					throw std::runtime_error("Unknown var kind.");
			}
		}
		
	}
	
}

