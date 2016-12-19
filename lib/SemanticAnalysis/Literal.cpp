#include <assert.h>
#include <stdio.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Constant.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/APInt.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		class InvalidLiteralSpecifierDiag: public Error {
		public:
			InvalidLiteralSpecifierDiag(const String specifier)
			: specifier_(specifier) { }
			
			std::string toString() const {
				return makeString("invalid literal specifier '%s'",
				                  specifier_.c_str());
			}
			
		private:
			String specifier_;
			
		};
		
		String integerSpecifierType(Context& context, const String& specifier,
		                            const Debug::SourceLocation& location) {
			if (specifier == "i8") {
				return context.getCString("int8_t");
			} else if (specifier == "i16") {
				return context.getCString("int16_t");
			} else if (specifier == "i32") {
				return context.getCString("int32_t");
			} else if (specifier == "i64") {
				return context.getCString("int64_t");
			} else if (specifier == "u8") {
				return context.getCString("uint8_t");
			} else if (specifier == "u16") {
				return context.getCString("uint16_t");
			} else if (specifier == "u32") {
				return context.getCString("uint32_t");
			} else if (specifier == "u64") {
				return context.getCString("uint64_t");
			}
			
			context.issueDiag(InvalidLiteralSpecifierDiag(specifier), location);
			return context.getCString("int64_t");
		}
		
		APInt integerMax(const String& typeName) {
			if (typeName == "int8_t") {
				return std::numeric_limits<int8_t>::max();
			} else if (typeName == "int16_t") {
				return std::numeric_limits<int16_t>::max();
			} else if (typeName == "int32_t") {
				return std::numeric_limits<int32_t>::max();
			} else if (typeName == "int64_t") {
				return std::numeric_limits<int64_t>::max();
			} else if (typeName == "uint8_t") {
				return std::numeric_limits<uint8_t>::max();
			} else if (typeName == "uint16_t") {
				return std::numeric_limits<uint16_t>::max();
			} else if (typeName == "uint32_t") {
				return std::numeric_limits<uint32_t>::max();
			} else if (typeName == "uint64_t") {
				return std::numeric_limits<uint64_t>::max();
			}
			
			locic_unreachable("Invalid integer type.");
		}
		
		class InvalidLiteralExceedsSpecifierMaximumDiag: public Error {
		public:
			InvalidLiteralExceedsSpecifierMaximumDiag(const String specifier)
			: specifier_(specifier) { }
			
			std::string toString() const {
				return makeString("integer literal  exceeds maximum of specifier '%s'",
				                  specifier_.c_str());
			}
			
		private:
			String specifier_;
			
		};
		
		class InvalidLiteralTooLargeForFixedWidthTypeDiag: public Error {
		public:
			InvalidLiteralTooLargeForFixedWidthTypeDiag() { }
			
			std::string toString() const {
				return "integer literal is too large to be represented in a fixed width type";
			}
			
		};
		
		String getIntegerConstantType(Context& context, const String& specifier, const Constant& constant,
		                              const Debug::SourceLocation& location) {
			assert(constant.kind() == Constant::INTEGER);
			
			const auto& integerValue = constant.integerValue();
			
			// Use a specifier if available.
			if (!specifier.empty() && specifier != "u") {
				const auto typeName = integerSpecifierType(context, specifier, location);
				const auto typeMax = integerMax(typeName);
				if (integerValue > typeMax) {
					context.issueDiag(InvalidLiteralExceedsSpecifierMaximumDiag(specifier),
					                  location);
				}
				return typeName;
			}
			
			// Otherwise determine type based on value.
			std::vector<String> types;
			types.push_back(context.getCString("int8_t"));
			types.push_back(context.getCString("int16_t"));
			types.push_back(context.getCString("int32_t"));
			types.push_back(context.getCString("int64_t"));
			
			for (const auto& typeName: types) {
				const auto specTypeName = specifier + typeName;
				if (integerValue <= integerMax(specTypeName)) {
					return specTypeName;
				}
			}
			
			context.issueDiag(InvalidLiteralTooLargeForFixedWidthTypeDiag(),
			                  location);
			return types.back();
		}
		
		class InvalidFloatingPointLiteralSpecifierDiag: public Error {
		public:
			InvalidFloatingPointLiteralSpecifierDiag(const String specifier)
			: specifier_(specifier) { }
			
			std::string toString() const {
				return makeString("invalid floating point literal specifier '%s'",
				                  specifier_.c_str());
			}
			
		private:
			String specifier_;
			
		};
		
		String getFloatingPointConstantType(Context& context, const String& specifier, const Constant& constant,
		                                    const Debug::SourceLocation& location) {
			assert(constant.kind() == Constant::FLOATINGPOINT);
			(void) constant;
			
			if (specifier == "f") {
				return context.getCString("float_t");
			} else if (specifier.empty() || specifier == "d") {
				return context.getCString("double_t");
			} else {
				context.issueDiag(InvalidFloatingPointLiteralSpecifierDiag(specifier),
				                  location);
				return context.getCString("double_t");
			}
		}
		
		class InvalidCharacterLiteralSpecifierDiag: public Error {
		public:
			InvalidCharacterLiteralSpecifierDiag(const String specifier)
			: specifier_(specifier) { }
			
			std::string toString() const {
				return makeString("invalid character literal specifier '%s'",
				                  specifier_.c_str());
			}
			
		private:
			String specifier_;
			
		};
		
		String getLiteralTypeName(Context& context, const String& specifier, const Constant& constant,
		                          const Debug::SourceLocation& location) {
			switch (constant.kind()) {
				case Constant::NULLVAL: {
					assert(specifier.empty());
					return context.getCString("null_t");
				}
				case Constant::BOOLEAN: {
					assert(specifier.empty());
					return context.getCString("bool_t");
				}
				case Constant::INTEGER: {
					return getIntegerConstantType(context, specifier, constant, location);
				}
				case Constant::FLOATINGPOINT: {
					return getFloatingPointConstantType(context, specifier, constant,
					                                    location);
				}
				case Constant::CHARACTER: {
					if (specifier == "") {
						return context.getCString("unichar_t");
					} else if (specifier == "C") {
						return context.getCString("ubyte_t");
					} else {
						context.issueDiag(InvalidCharacterLiteralSpecifierDiag(specifier),
						                  location);
						return context.getCString("ubyte_t");
					}
				}
				case Constant::STRING: {
					locic_unreachable("String constants should already be handled.");
				}
			}
			
			locic_unreachable("Unknown constant kind.");
		}
		
		const AST::Type* getLiteralType(Context& context, const String& specifier, const Constant& constant,
		                                const Debug::SourceLocation& location) {
			switch (constant.kind()) {
				case Constant::STRING: {
					// C strings have the type 'const ubyte *', as opposed to just a
					// type name, so their type needs to be generated specially.
					const auto byteType = getBuiltInType(context, context.getCString("ubyte_t"), {});
					
					// Generate type 'const ubyte'.
					const auto constByteType = byteType->createTransitiveConstType(SEM::Predicate::True());
					
					// Generate type 'const ubyte *'.
					return getBuiltInType(context, context.getCString("ptr_t"), { constByteType });
				}
				default: {
					const auto typeName = getLiteralTypeName(context, specifier, constant, location);
					return getBuiltInType(context, typeName, {});
				}
			}
		}
		
		class NoFunctionForStringLiteralSpecifierDiag: public Error {
		public:
			NoFunctionForStringLiteralSpecifierDiag(const String function, const String specifier)
			: function_(function), specifier_(specifier) { }
			
			std::string toString() const {
				return makeString("cannot find function '%s' for string literal specifier '%s'",
				                  function_.c_str(), specifier_.c_str());
			}
			
		private:
			String function_;
			String specifier_;
			
		};
		
		SEM::Value getLiteralValue(Context& context, const String& specifier, const Constant& constant, const Debug::SourceLocation& location) {
			auto constantValue = SEM::Value::Constant(constant, getLiteralType(context, specifier, constant, location));
			
			if (constant.kind() != Constant::STRING || specifier == "C") {
				return constantValue;
			}
			
			const auto functionName = context.getCString("string_literal") + (!specifier.empty() ? context.getCString("_") + specifier : context.getCString(""));
			
			const auto searchResult = performSearch(context, Name::Absolute() + functionName);
			if (!searchResult.isFunction()) {
				context.issueDiag(NoFunctionForStringLiteralSpecifierDiag(functionName, specifier),
				                  location);
				return constantValue;
			}
			
			auto& typeBuilder = context.typeBuilder();
			const auto functionRefType = typeBuilder.getFunctionPointerType(searchResult.function().type());
			
			auto functionRef = SEM::Value::FunctionRef(nullptr, searchResult.function(), {}, functionRefType);
			return CallValue(context, std::move(functionRef), makeHeapArray( std::move(constantValue) ), location);
		}
		
	}
	
}


