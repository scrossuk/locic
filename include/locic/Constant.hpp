#ifndef LOCIC_CONSTANT_HPP
#define LOCIC_CONSTANT_HPP

#include <stdint.h>
#include <cassert>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/Hasher.hpp>
#include <locic/Support/String.hpp>

namespace locic{
	
	class Constant {
		public:
			enum Kind {
				NULLVAL,
				BOOLEAN,
				INTEGER,
				FLOATINGPOINT,
				CHARACTER,
				STRING
			};
			
			typedef unsigned long long IntegerVal;
			typedef long double FloatVal;
			typedef uint32_t CharVal;
			
			static Constant Null(){
				return Constant(NULLVAL);
			}
			
			static Constant True(){
				Constant constant(BOOLEAN);
				constant.bool_ = true;
				return constant;
			}
			
			static Constant False(){
				Constant constant(BOOLEAN);
				constant.bool_ = false;
				return constant;
			}
			
			static Constant Integer(IntegerVal value){
				Constant constant(INTEGER);
				constant.integer_ = value;
				return constant;
			}
			
			static Constant Float(FloatVal value){
				Constant constant(FLOATINGPOINT);
				constant.float_ = value;
				return constant;
			}
			
			static Constant Character(CharVal value){
				Constant constant(CHARACTER);
				constant.character_ = value;
				return constant;
			}
			
			static Constant StringVal(const String value){
				Constant constant(STRING);
				constant.string_ = value;
				return constant;
			}
			
			// Default (non-initialising!) constructor allows
			// placing this type in unions.
			Constant() = default;
			
			Kind kind() const{
				return kind_;
			}
			
			bool isNull() const {
				return kind() == NULLVAL;
			}
			
			bool isBool() const {
				return kind() == BOOLEAN;
			}
			
			bool isInteger() const {
				return kind() == INTEGER;
			}
			
			bool isFloat() const {
				return kind() == FLOATINGPOINT;
			}
			
			bool isCharacter() const {
				return kind() == CHARACTER;
			}
			
			bool isString() const {
				return kind() == STRING;
			}
			
			bool boolValue() const{
				assert(kind_ == BOOLEAN);
				return bool_;
			}
			
			IntegerVal integerValue() const{
				assert(kind_ == INTEGER);
				return integer_;
			}
			
			FloatVal floatValue() const{
				assert(kind_ == FLOATINGPOINT);
				return float_;
			}
			
			CharVal characterValue() const{
				assert(kind_ == CHARACTER);
				return character_;
			}
			
			const String& stringValue() const{
				assert(kind_ == STRING);
				return string_;
			}
			
			bool operator==(const Constant& other) const {
				if (kind() != other.kind()) {
					return false;
				}
				
				switch (kind()) {
					case NULLVAL:
						return true;
					case BOOLEAN:
						return boolValue() == other.boolValue();
					case INTEGER:
						return integerValue() == other.integerValue();
					case FLOATINGPOINT:
						return floatValue() == other.floatValue();
					case CHARACTER:
						return characterValue() == other.characterValue();
					case STRING:
						return stringValue() == other.stringValue();
				}
				
				return false;
			}
			
			size_t hash() const {
				Hasher hasher;
				hasher.add(kind());
				
				switch (kind()) {
					case NULLVAL:
						break;
					case BOOLEAN:
						hasher.add(boolValue());
						break;
					case INTEGER:
						hasher.add(integerValue());
						break;
					case FLOATINGPOINT:
						hasher.add(floatValue());
						break;
					case CHARACTER:
						hasher.add(characterValue());
						break;
					case STRING:
						hasher.add(stringValue());
						break;
				}
				
				return hasher.get();
			}
			
			std::string toString() const {
				switch (kind_) {
					case NULLVAL:
						return "NullConstant";
					case BOOLEAN:
						return makeString("BoolConstant(%s)", bool_ ? "true" : "false");
					case INTEGER:
						return makeString("IntegerConstant(%llu)", integerValue());
					case FLOATINGPOINT:
						return makeString("FloatConstant(%Lf)", floatValue());
					case CHARACTER:
						return makeString("CharacterConstant(%llu)",
						                  static_cast<unsigned long long>(characterValue()));
					case STRING:
						return makeString("StringConstant(\"%s\")", escapeString(stringValue().asStdString()).c_str());
				}
				
				return "[UNKNOWN CONSTANT]";
			}
			
		private:
			Constant(Kind pKind)
			: kind_(pKind) { }
			
			Kind kind_;
			
			union{
				bool bool_;
				IntegerVal integer_;
				FloatVal float_;
				CharVal character_;
				String string_;
			};
		
	};

}

#endif
