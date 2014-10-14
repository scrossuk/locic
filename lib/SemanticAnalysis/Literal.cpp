#include <assert.h>
#include <stdio.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		std::string integerSpecifierType(const std::string& specifier) {
			if (specifier == "i8") {
				return "int8_t";
			} else if (specifier == "i16") {
				return "int16_t";
			} else if (specifier == "i32") {
				return "int32_t";
			} else if (specifier == "i64") {
				return "int64_t";
			} else if (specifier == "u8") {
				return "uint8_t";
			} else if (specifier == "u16") {
				return "uint16_t";
			} else if (specifier == "u32") {
				return "uint32_t";
			} else if (specifier == "u64") {
				return "uint64_t";
			}
			
			throw ErrorException(makeString("Invalid integer literal specifier '%s'.", specifier.c_str()));
		}
		
		unsigned long long integerMax(const std::string& typeName) {
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
		
		std::string getIntegerConstantType(const std::string& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::INTEGER);
			
			const auto integerValue = constant.integerValue();
			
			// Use a specifier if available.
			if (!specifier.empty() && specifier != "u") {
				const auto typeName = integerSpecifierType(specifier);
				const auto typeMax = integerMax(typeName);
				if (integerValue > typeMax) {
					throw ErrorException(makeString("Integer literal '%llu' exceeds maximum of specifier '%s'.",
						(unsigned long long) integerValue, specifier.c_str()));
				}
				return typeName;
			}
			
			// Otherwise determine type based on value.
			std::vector<std::string> types;
			types.push_back("int8_t");
			types.push_back("int16_t");
			types.push_back("int32_t");
			types.push_back("int64_t");
			
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
		
		std::string getFloatingPointConstantType(const std::string& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::FLOATINGPOINT);
			(void) constant;
			
			if (specifier == "f") {
				return "float_t";
			} else if (specifier.empty() || specifier == "d") {
				return "double_t";
			} else {
				throw ErrorException(makeString("Invalid floating point literal specifier '%s'.",
					specifier.c_str()));
			}
		}
		
		std::string getLiteralTypeName(const std::string& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::NULLVAL: {
					if (specifier.empty()) {
						return "null_t";
					} else {
						throw ErrorException(makeString("Invalid null literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::BOOLEAN: {
					if (specifier.empty()) {
						return "bool";
					} else {
						throw ErrorException(makeString("Invalid boolean literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::INTEGER: {
					return getIntegerConstantType(specifier, constant);
				}
				case Constant::FLOATINGPOINT: {
					return getFloatingPointConstantType(specifier, constant);
				}
				case Constant::CHARACTER: {
					if (specifier == "") {
						return "unichar";
					} else if (specifier == "C") {
						return "ubyte_t";
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
		
		SEM::Type* getLiteralType(Context& context, const std::string& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::STRING: {
					// C strings have the type 'const ubyte * const', as opposed to just a
					// type name, so their type needs to be generated specially.
					const auto byteType = getBuiltInType(context.scopeStack(), "ubyte_t");
					const auto ptrTypeInstance = getBuiltInType(context.scopeStack(), "__ptr")->getObjectType();
					
					// Generate type 'const ubyte'.
					const auto constByteType = byteType->createConstType();
					
					// Generate type 'const ptr<const ubyte>'.
					return SEM::Type::Object(ptrTypeInstance, { constByteType })->createConstType();
				}
				default: {
					const auto typeName = getLiteralTypeName(specifier, constant);
					return getBuiltInType(context.scopeStack(), typeName);
				}
			}
		}
		
		SEM::Value* getLiteralValue(Context& context, const std::string& specifier, const Constant& constant, const Debug::SourceLocation& location) {
			const auto constantValue = SEM::Value::Constant(&constant, getLiteralType(context, specifier, constant));
			
			if (constant.kind() != Constant::STRING || specifier == "C") {
				return constantValue;
			}
			
			const auto functionName = std::string("string_literal") + (!specifier.empty() ? std::string("_") + specifier : std::string(""));
			
			const auto searchResult = performSearch(context, Name::Absolute() + functionName);
			if (!searchResult.isFunction()) {
				throw ErrorException(makeString("Invalid string literal specifier '%s' at %s; failed to find relevant function '%s'.",
					specifier.c_str(), location.toString().c_str(), functionName.c_str()));
			}
			
			const auto functionRef = SEM::Value::FunctionRef(nullptr, searchResult.function(), {}, SEM::TemplateVarMap());
			return CallValue(context, functionRef, { constantValue }, location);
		}
		
	}
	
}


