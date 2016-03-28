#ifndef LOCIC_CONSTANT_HPP
#define LOCIC_CONSTANT_HPP

#include <stdint.h>
#include <cassert>

#include <locic/Support/APInt.hpp>
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
			
			static Constant Integer(APInt value){
				Constant constant(INTEGER);
				constant.integer_ = std::move(value);
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
			
			Constant() : kind_(NULLVAL) { }
			
			Constant(const Constant& other)
			: kind_(other.kind_) {
				switch (other.kind()) {
					case NULLVAL:
						break;
					case BOOLEAN:
						bool_ = other.boolValue();
						break;
					case INTEGER:
						integer_ = other.integerValue().copy();
						break;
					case FLOATINGPOINT:
						float_ = other.floatValue();
						break;
					case CHARACTER:
						character_ = other.characterValue();
						break;
					case STRING:
						string_ = other.stringValue();
						break;
				}
			}
			
			Constant(Constant&&) = default;
			Constant& operator=(Constant&&) = default;
			
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
			
			const APInt& integerValue() const{
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
						return makeString("IntegerConstant(%s)", integerValue().toString().c_str());
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
			APInt integer_;
			
			union{
				bool bool_;
				FloatVal float_;
				CharVal character_;
				String string_;
			};
		
	};

}

#endif
