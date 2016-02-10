#ifndef LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include <locic/Exception.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Exception: public locic::Exception {
		protected:
			Exception() = default;
			Exception(const Exception&) = default;
			Exception& operator=(const Exception&) = default;
			~Exception() { }
		};
		
		class CastException: public Exception {
		protected:
			CastException() = default;
			CastException(const CastException&) = default;
			CastException& operator=(const CastException&) = default;
			~CastException() { }
		};
		
		class NameClashException final: public Exception {
			public:
				enum Kind {
					FUNCTION_WITH_FUNCTION,
					FUNCTION_WITH_NAMESPACE,
					FUNCTION_WITH_TYPE,
					TYPE_WITH_NAMESPACE,
					TYPE_WITH_TYPE
				};
				
				NameClashException(Kind kind, Name name)
					: kind_(kind), name_(std::move(name)) { }
				
				Kind getKind() const {
					return kind_;
				}
				
				const Name& getName() const {
					return name_;
				}
				
				std::string toString() const {
					switch(kind_) {
						case FUNCTION_WITH_FUNCTION: {
							return makeString("Function name '%s' clashes with existing function.", name_.toString().c_str());
						}
						case FUNCTION_WITH_NAMESPACE: {
							return makeString("Function name '%s' clashes with existing namespace.", name_.toString().c_str());
						}
						case FUNCTION_WITH_TYPE: {
							return makeString("Function name '%s' clashes with existing type.", name_.toString().c_str());
						}
						case TYPE_WITH_NAMESPACE: {
							return makeString("Type name '%s' clashes with existing namespace.", name_.toString().c_str());
						}
						case TYPE_WITH_TYPE: {
							return makeString("Type name '%s' clashes with existing type.", name_.toString().c_str());
						}
						default: {
							assert(false);
							return "";
						}
					}
				}
				
			private:
				Kind kind_;
				Name name_;
				
		};
		
		class TemplateVariableClashException final: public Exception {
			public:
				TemplateVariableClashException(Name typeName, String varName)
					: typeName_(std::move(typeName)), varName_(std::move(varName)) { }
					
				std::string toString() const {
					return makeString("More than one template variable shares name '%s' in type '%s'.", varName_.c_str(), typeName_.toString().c_str());
				}
				
			private:
				Name typeName_;
				String varName_;
				
		};
		
		class MemberVariableClashException final: public Exception {
			public:
				MemberVariableClashException(Name typeName, String varName)
					: typeName_(std::move(typeName)), varName_(std::move(varName)) { }
					
				std::string toString() const {
					return makeString("More than one member variable shares name '%s' in type '%s'.", varName_.c_str(), typeName_.toString().c_str());
				}
				
			private:
				Name typeName_;
				String varName_;
				
		};
		
		class NonUnifiableTypeClashException final: public Exception {
			public:
				NonUnifiableTypeClashException(Name name)
					: name_(std::move(name)) { }
					
				std::string toString() const {
					return makeString("Non-unifiable types share name '%s'.", name_.toString().c_str());
				}
				
			private:
				Name name_;
				
		};
		
		class NonUnifiableFunctionsException final: public Exception {
			public:
				NonUnifiableFunctionsException(Name name, String newType, String existingType)
					: name_(std::move(name)), newType_(std::move(newType)),
					  existingType_(std::move(existingType)) { }
					  
				std::string toString() const {
					return makeString("Declarations of function '%s' don't match: %s and %s.",
							name_.toString().c_str(),
							newType_.c_str(),
							existingType_.c_str());
				}
				
			private:
				Name name_;
				String newType_;
				String existingType_;
				
		};
		
		class MultipleFunctionDefinitionsException final: public Exception {
			public:
				MultipleFunctionDefinitionsException(Name name)
					: name_(std::move(name)) { }
					
				std::string toString() const {
					return makeString("Function '%s' is defined more than once.", name_.toString().c_str());
				}
				
			private:
				Name name_;
				
		};
		
		class ParamVariableClashException final: public Exception {
			public:
				ParamVariableClashException(Name functionName, String paramName)
					: functionName_(std::move(functionName)), paramName_(std::move(paramName)) { }
					
				std::string toString() const {
					return makeString("Parameter variable '%s' is defined more than once, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				String paramName_;
				
		};
		
		class LocalVariableClashException final: public Exception {
			public:
				LocalVariableClashException(Name functionName, String paramName)
					: functionName_(std::move(functionName)), paramName_(std::move(paramName)) { }
					
				std::string toString() const {
					return makeString("Local variable '%s' is defined more than once, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				String paramName_;
				
		};
		
		class MissingReturnStatementException final: public Exception {
			public:
				MissingReturnStatementException(Name functionName)
					: functionName_(std::move(functionName)) { }
					
				std::string toString() const {
					return makeString("Control reaches end of function '%s' with non-void return type; it needs a return statement.", functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				
		};
		
		class ParamVoidTypeException final: public Exception {
			public:
				ParamVoidTypeException(Name functionName, String paramName)
					: functionName_(std::move(functionName)), paramName_(std::move(paramName)) { }
					
				std::string toString() const {
					return makeString("Parameter variable '%s' cannot have void type, in function '%s'.", paramName_.c_str(), functionName_.toString().c_str());
				}
				
			private:
				Name functionName_;
				String paramName_;
				
		};
		
		class LocalVariableShadowingException final: public Exception {
			public:
				LocalVariableShadowingException(String varName)
					: varName_(std::move(varName)) { }
					
				std::string toString() const {
					return makeString("Local variable '%s' shadows existing local variable of the same name.", varName_.c_str());
				}
				
			private:
				String varName_;
				
		};
		
		class ErrorException final: public Exception {
			public:
				ErrorException(const std::string& message)
					: message_(message) { }
					
				std::string toString() const {
					return message_;
				}
				
			private:
				std::string message_;
				
		};
		
	}
	
}

#endif
