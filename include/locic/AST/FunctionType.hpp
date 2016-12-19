#ifndef LOCIC_AST_FUNCTIONTYPE_HPP
#define LOCIC_AST_FUNCTIONTYPE_HPP

#include <string>

#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TemplateVarMap.hpp>

#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/ValueArray.hpp>

namespace locic {
	
	namespace SEM {
		
		class Context;
		class Type;
		
	}
	
	namespace AST {
		
		class FunctionAttributes {
		public:
			FunctionAttributes(bool isVarArg, bool isMethod,
			                   bool isTemplated,
			                   SEM::Predicate noExceptPredicate);
			
			FunctionAttributes copy() const;
			
			bool isVarArg() const;
			
			/**
			 * \brief Query whether the function is a dynamic method.
			 * 
			 * A function is a dynamic method if it has a 'this'
			 * argument; note this does not include static methods.
			 */
			bool isMethod() const;
			
			/**
			 * \brief Query whether the function is templated.
			 * 
			 * A function is templated if either it or its parent
			 * type accepts one or more non-virtual templates.
			 */
			bool isTemplated() const;
			
			const SEM::Predicate& noExceptPredicate() const;
			
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
			SEM::Predicate noExceptPredicate_;
			
		};
		
		class FunctionTypeData {
		public:
			FunctionTypeData(FunctionAttributes attributes,
			                 const SEM::Type* returnType,
			                 SEM::TypeArray parameterTypes);
			
			FunctionTypeData copy() const;
			
			const SEM::Context& context() const;
			
			const FunctionAttributes& attributes() const;
			
			const SEM::Type* returnType() const;
			
			const SEM::TypeArray& parameterTypes() const;
			
			std::string toString() const;
			
			std::string nameToString() const;
			
			std::size_t hash() const;
			
			bool operator==(const FunctionTypeData& other) const;
			
			bool operator!=(const FunctionTypeData& other) const {
				return !(*this == other);
			}
			
		private:
			FunctionAttributes attributes_;
			const SEM::Type* returnType_;
			SEM::TypeArray parameterTypes_;
			
		};
		
		class FunctionType {
		public:
			FunctionType()
			: data_(nullptr) { }
			
			FunctionType(FunctionAttributes attributes,
			             const SEM::Type* returnType,
			             SEM::TypeArray parameterTypes);
			
			FunctionType(const FunctionTypeData& data)
			: data_(&data) { }
			
			const SEM::Context& context() const {
				return data_->context();
			}
			
			const FunctionAttributes& attributes() const {
				return data_->attributes();
			}
			
			bool isVarArg() const {
				return attributes().isVarArg();
			}
			
			const SEM::Type* returnType() const {
				return data_->returnType();
			}
			
			const SEM::TypeArray& parameterTypes() const {
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
