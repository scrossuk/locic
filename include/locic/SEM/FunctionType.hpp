#ifndef LOCIC_SEM_FUNCTIONTYPE_HPP
#define LOCIC_SEM_FUNCTIONTYPE_HPP

#include <string>

#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/ValueArray.hpp>

namespace locic {
	
	namespace SEM {
		
		class Context;
		class Type;
		
		class FunctionAttributes {
		public:
			FunctionAttributes(bool isVarArg, bool isMethod, bool isTemplated, Predicate noExceptPredicate);
			
			FunctionAttributes copy() const;
			
			bool isVarArg() const;
			
			bool isMethod() const;
			
			bool isTemplated() const;
			
			const Predicate& noExceptPredicate() const;
			
			std::string toString() const;
			
			std::size_t hash() const;
			
			bool operator==(const FunctionAttributes& other) const;
			bool operator!=(const FunctionAttributes& other) const {
				return !(*this == other);
			}
			
		private:
			bool isVarArg_;
			bool isMethod_;
			bool isTemplated_;
			Predicate noExceptPredicate_;
			
		};
		
		class FunctionTypeData {
		public:
			FunctionTypeData(FunctionAttributes attributes, const Type* returnType, TypeArray parameterTypes);
			
			FunctionTypeData copy() const;
			
			const Context& context() const;
			
			const FunctionAttributes& attributes() const;
			
			const Type* returnType() const;
			
			const TypeArray& parameterTypes() const;
			
			std::string toString() const;
			
			std::string nameToString() const;
			
			std::size_t hash() const;
			
			bool operator==(const FunctionTypeData& other) const;
			
			bool operator!=(const FunctionTypeData& other) const {
				return !(*this == other);
			}
			
		private:
			FunctionAttributes attributes_;
			const Type* returnType_;
			TypeArray parameterTypes_;
			
		};
		
		class FunctionType {
		public:
			FunctionType()
			: data_(nullptr) { }
			
			FunctionType(FunctionAttributes attributes, const Type* returnType, TypeArray parameterTypes);
			
			FunctionType(const FunctionTypeData& data)
			: data_(&data) { }
			
			const Context& context() const {
				return data_->context();
			}
			
			const FunctionAttributes& attributes() const {
				return data_->attributes();
			}
			
			bool isVarArg() const {
				return attributes().isVarArg();
			}
			
			const Type* returnType() const {
				return data_->returnType();
			}
			
			const TypeArray& parameterTypes() const {
				return data_->parameterTypes();
			}
			
			FunctionType substitute(const TemplateVarMap& templateVarMap) const;
			
			FunctionType makeTemplated() const;
			
			bool dependsOnAny(const TemplateVarArray& array) const;
			
			bool dependsOnOnly(const TemplateVarArray& array) const;
			
			std::string toString() const {
				return data_->toString();
			}
			
			std::string nameToString() const {
				return data_->nameToString();
			}
			
			std::size_t hash() const;
			
			bool operator==(const FunctionType& other) const {
				return data_ == other.data_;
			}
			
			bool operator!=(const FunctionType& other) const {
				return data_ != other.data_;
			}
			
		private:
			const FunctionTypeData* data_;
			
		};
		
	}
	
}

#endif
