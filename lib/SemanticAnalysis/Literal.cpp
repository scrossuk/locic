#include <assert.h>
#include <stdio.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		String integerSpecifierType(Context& context, const String& specifier) {
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
			
			throw ErrorException(makeString("Invalid integer literal specifier '%s'.", specifier.c_str()));
		}
		
		unsigned long long integerMax(const String& typeName) {
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
			
			throw std::runtime_error(makeString("Invalid integer type '%s'.", typeName.c_str()));
		}
		
		String getIntegerConstantType(Context& context, const String& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::INTEGER);
			
			const auto integerValue = constant.integerValue();
			
			// Use a specifier if available.
			if (!specifier.empty() && specifier != "u") {
				const auto typeName = integerSpecifierType(context, specifier);
				const auto typeMax = integerMax(typeName);
				if (integerValue > typeMax) {
					throw ErrorException(makeString("Integer literal '%llu' exceeds maximum of specifier '%s'.",
						(unsigned long long) integerValue, specifier.c_str()));
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
				// TODO: use arbitary-precision arithmetic.
				if (integerValue <= integerMax(specTypeName)) {
					return specTypeName;
				}
			}
			
			throw ErrorException(makeString("Integer literal '%llu' is too large to be represented in a fixed width type.",
				integerValue));
		}
		
		String getFloatingPointConstantType(Context& context, const String& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::FLOATINGPOINT);
			(void) constant;
			
			if (specifier == "f") {
				return context.getCString("float_t");
			} else if (specifier.empty() || specifier == "d") {
				return context.getCString("double_t");
			} else {
				throw ErrorException(makeString("Invalid floating point literal specifier '%s'.",
					specifier.c_str()));
			}
		}
		
		String getLiteralTypeName(Context& context, const String& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::NULLVAL: {
					if (specifier.empty()) {
						return context.getCString("null_t");
					} else {
						throw ErrorException(makeString("Invalid null literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::BOOLEAN: {
					if (specifier.empty()) {
						return context.getCString("bool");
					} else {
						throw ErrorException(makeString("Invalid boolean literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::INTEGER: {
					return getIntegerConstantType(context, specifier, constant);
				}
				case Constant::FLOATINGPOINT: {
					return getFloatingPointConstantType(context, specifier, constant);
				}
				case Constant::CHARACTER: {
					if (specifier == "") {
						return context.getCString("unichar");
					} else if (specifier == "C") {
						return context.getCString("ubyte_t");
					} else {
						throw ErrorException(makeString("Invalid character literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::STRING: {
					// Not handled here.
					std::terminate();
				}
			}
			
			std::terminate();
		}
		
		const SEM::Type* getLiteralType(Context& context, const String& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::STRING: {
					// C strings have the type 'const ubyte *', as opposed to just a
					// type name, so their type needs to be generated specially.
					const auto byteType = getBuiltInType(context, context.getCString("ubyte_t"), {});
					
					// Generate type 'const ubyte'.
					const auto constByteType = byteType->createTransitiveConstType(SEM::Predicate::True());
					
					// Generate type 'const ubyte *'.
					return getBuiltInType(context, context.getCString("__ptr"), { constByteType });
				}
				default: {
					const auto typeName = getLiteralTypeName(context, specifier, constant);
					return getBuiltInType(context, typeName, {});
				}
			}
		}
		
		SEM::Value getLiteralValue(Context& context, const String& specifier, const Constant& constant, const Debug::SourceLocation& location) {
			auto constantValue = SEM::Value::Constant(constant, getLiteralType(context, specifier, constant));
			
			if (constant.kind() != Constant::STRING || specifier == "C") {
				return constantValue;
			}
			
			const auto functionName = context.getCString("string_literal") + (!specifier.empty() ? context.getCString("_") + specifier : context.getCString(""));
			
			const auto searchResult = performSearch(context, Name::Absolute() + functionName);
			if (!searchResult.isFunction()) {
				throw ErrorException(makeString("Invalid string literal specifier '%s' at %s; failed to find relevant function '%s'.",
					specifier.c_str(), location.toString().c_str(), functionName.c_str()));
			}
			
			auto& typeBuilder = context.typeBuilder();
			const auto functionRefType = typeBuilder.getFunctionPointerType(searchResult.function().type());
			
			auto functionRef = SEM::Value::FunctionRef(nullptr, &(searchResult.function()), {}, functionRefType);
			return CallValue(context, std::move(functionRef), makeHeapArray( std::move(constantValue) ), location);
		}
		
	}
	
}


