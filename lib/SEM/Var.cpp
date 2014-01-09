#include <assert.h>
#include <stdexcept>
#include <string>

#include <locic/String.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Var* Var::Any() {
			Var* var = new Var;
			var->kind_ = ANY;
			return var;
		}
		
		Var* Var::Basic(Type* type) {
			Var* var = new Var;
			var->kind_ = BASIC;
			var->type_ = type;
			return var;
		}
		
		Var* Var::Composite(Type* parent, const std::vector<Var*>& children) {
			Var* var = new Var;
			var->kind_ = COMPOSITE;
			var->type_ = parent;
			var->children_ = children;
			return var;
		}
		
		Var::Var() : kind_(ANY), type_(NULL) { }
		
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
		
		Type* Var::type() const {
			assert(isBasic() || isComposite());
			return type_;
		}
		
		const std::vector<Var*>& Var::children() const {
			assert(isComposite());
			return children_;
		}
		
		Var* Var::substitute(const Map<TemplateVar*, Type*>& templateVarMap) const {
			switch (kind()) {
				case ANY:
					return Var::Any();
				case BASIC:
					return Var::Basic(type()->substitute(templateVarMap));
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
					return "Var[ANY]()";
				case BASIC:
					return makeString("Var[BASIC](type: %s)", type()->toString().c_str());
				case COMPOSITE:
					return makeString("Var[COMPOSITE](type: %s, children: %s)", type()->toString().c_str(),
						makeArrayString(children()).c_str());
				default:
					throw std::runtime_error("Unknown var kind.");
			}
		}
		
	}
	
}

