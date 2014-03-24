#ifndef LOCIC_CONSTANT_HPP
#define LOCIC_CONSTANT_HPP

#include <stdint.h>
#include <cassert>
#include <boost/optional.hpp>

#include <locic/String.hpp>

namespace locic{

	class Constant{
		public:
			enum Kind {
				NULLVAL,
				BOOLEAN,
				INTEGER,
				FLOATINGPOINT,
				STRING
			};
			
			enum IntegerKind {
				SIGNED,
				UNSIGNED
			};
			
			enum FloatKind {
				FLOAT,
				DOUBLE
			};
	
			enum StringKind {
				C_STRING,
				LOCI_STRING
			};
			
			typedef unsigned long long IntegerVal;
			typedef long double FloatVal;
			
			static inline Constant * Null(){
				return new Constant(NULLVAL);
			}
			
			static inline Constant * True(){
				Constant * constant = new Constant(BOOLEAN);
				constant->bool_ = true;
				return constant;
			}
			
			static inline Constant * False(){
				Constant * constant = new Constant(BOOLEAN);
				constant->bool_ = false;
				return constant;
			}
			
			static inline Constant * Integer(IntegerKind kind, IntegerVal value){
				Constant * constant = new Constant(INTEGER);
				constant->integerKind_ = kind;
				constant->integer_ = value;
				return constant;
			}
			
			static inline Constant * Float(FloatKind kind, FloatVal value){
				Constant * constant = new Constant(FLOATINGPOINT);
				constant->floatKind_ = kind;
				constant->float_ = value;
				return constant;
			}
			
			static inline Constant * String(StringKind kind, const std::string& value){
				Constant * constant = new Constant(STRING);
				constant->stringKind_ = kind;
				constant->string_ = value;
				return constant;
			}
			
			inline Kind kind() const{
				return kind_;
			}
			
			inline bool boolValue() const{
				assert(kind_ == BOOLEAN);
				return bool_;
			}
			
			inline IntegerKind integerKind() const{
				assert(kind_ == INTEGER);
				return integerKind_;
			}
			
			inline IntegerVal integerValue() const{
				assert(kind_ == INTEGER);
				return integer_;
			}
			
			inline FloatKind floatKind() const{
				assert(kind_ == FLOATINGPOINT);
				return floatKind_;
			}
			
			inline FloatVal floatValue() const{
				assert(kind_ == FLOATINGPOINT);
				return float_;
			}
			
			inline StringKind stringKind() const{
				assert(kind_ == STRING);
				return stringKind_;
			}
			
			inline std::string stringValue() const{
				assert(kind_ == STRING);
				return string_;
			}
			
			inline std::string getTypeName() const{
				switch(kind_){
					case NULLVAL:
						return "null_t";
					case BOOLEAN:
						return "bool";
					case INTEGER:
						return "integer_literal_t";
					case FLOATINGPOINT:
						return "float_literal_t";
					case STRING:
						return "string";
					default:
						assert(false && "Unknown constant type");
						return "";
				}
			}
			
			std::string toString() const {
				switch (kind_) {
					case NULLVAL:
						return "NullConstant";
					case BOOLEAN:
						return makeString("BoolConstant(%s)", bool_ ? "true" : "false");
					case INTEGER:
					{
						const auto kindString = integerKind() == SIGNED ? "signed" : "unsigned";
						return makeString("IntegerConstant(%s, %lld)", kindString, integerValue());
					}
					case FLOATINGPOINT:
					{
						const auto kindString = floatKind() == FLOAT ? "float" : "double";
						return makeString("FloatConstant(%s, %Lf)", kindString, float_);
					}
					case STRING:
					{
						const auto kindString = stringKind() == C_STRING ? "c_string" : "string";
						return makeString("StringConstant(%s, \"%s\")", kindString, escapeString(string_).c_str());
					}
					default:
						return "[UNKNOWN CONSTANT]";
				}
			}
			
		private:
			inline Constant(Kind pKind)
				: kind_(pKind) { }
			
			Kind kind_;
		
			union {
				IntegerKind integerKind_;
				FloatKind floatKind_;
				StringKind stringKind_;
			};
			
			union{
				bool bool_;
				IntegerVal integer_;
				FloatVal float_;
			};
			
			std::string string_;
		
	};

}

#endif
