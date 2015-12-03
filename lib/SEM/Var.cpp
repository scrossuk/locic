#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/String.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		std::unique_ptr<Var> Var::Any(const Type* constructType) {
			std::unique_ptr<Var> var(new Var());
			var->kind_ = ANY;
			var->constructType_ = constructType;
			return var;
		}
		
		std::unique_ptr<Var> Var::Basic(const Type* constructType, const Type* type) {
			std::unique_ptr<Var> var(new Var());
			var->kind_ = BASIC;
			var->constructType_ = constructType;
			var->type_ = type;
			return var;
		}
		
		std::unique_ptr<Var> Var::Composite(const Type* type, std::vector<std::unique_ptr<Var>> children) {
			std::unique_ptr<Var> var(new Var());
			var->kind_ = COMPOSITE;
			var->constructType_ = type;
			var->type_ = type;
			var->children_ = std::move(children);
			return var;
		}
		
		Var::Var() : kind_(ANY), constructType_(NULL), type_(NULL),
		isUsed_(false), isMarkedUnused_(false), isOverrideConst_(false),
		index_(-1) { }
		
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
		
		const std::vector<std::unique_ptr<Var>>& Var::children() const {
			assert(isComposite());
			return children_;
		}
		
		bool Var::isUsed() const {
			return isUsed_;
		}
		
		void Var::setUsed() {
			isUsed_ = true;
		}
		
		bool Var::isMarkedUnused() const {
			return isMarkedUnused_;
		}
		
		void Var::setMarkedUnused(const bool argIsMarkedUnused) {
			isMarkedUnused_ = argIsMarkedUnused;
		}
		
		bool Var::isOverrideConst() const {
			return isOverrideConst_;
		}
		
		void Var::setOverrideConst(const bool argIsOverrideConst) {
			isOverrideConst_ = argIsOverrideConst;
		}
		
		size_t Var::index() const {
			assert(index_ != (size_t) -1);
			return index_;
		}
		
		void Var::setIndex(const size_t index) {
			assert(index_ == (size_t) -1);
			index_ = index;
		}
		
		void Var::setDebugInfo(const Debug::VarInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::VarInfo> Var::debugInfo() const {
			return debugInfo_;
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
							makeArrayPtrString(children()).c_str());
				default:
					throw std::runtime_error("Unknown var kind.");
			}
		}
		
	}
	
}

