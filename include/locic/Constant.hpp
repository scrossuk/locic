#ifndef LOCIC_CONSTANT_HPP
#define LOCIC_CONSTANT_HPP

#include <stdint.h>
#include <cassert>
#include <boost/optional.hpp>

#include <locic/String.hpp>

namespace locic{

	class Constant{
		public:
			enum Type{
				NULLVAL,
				BOOLEAN,
				INTEGER,
				FLOATINGPOINT,
				STRING
			};
	
			enum StringType{
				CSTRING,
				LOCISTRING
			};
			
			typedef long long IntegerVal;
			typedef unsigned long long UintVal;
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
			
			static inline Constant * Integer(IntegerVal value){
				Constant * constant = new Constant(INTEGER);
				constant->integer_ = value;
				return constant;
			}
			
			static inline Constant * Float(FloatVal value){
				Constant * constant = new Constant(FLOATINGPOINT);
				constant->float_ = value;
				return constant;
			}
			
			static inline Constant * String(const std::string& value){
				Constant * constant = new Constant(STRING);
				constant->string_ = value;
				return constant;
			}
			
			inline Type getType() const{
				return type_;
			}
			
			inline bool getBool() const{
				assert(type_ == BOOLEAN);
				return bool_;
			}
			
			inline IntegerVal getInteger() const{
				assert(type_ == INTEGER);
				return integer_;
			}
			
			inline FloatVal getFloat() const{
				assert(type_ == FLOATINGPOINT);
				return float_;
			}
			
			inline std::string getString() const{
				assert(type_ == STRING);
				return string_;
			}
			
			inline std::string getTypeName() const{
				switch(type_){
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
				switch(type_){
					case NULLVAL:
						return "NullConstant";
					case BOOLEAN:
						return makeString("BoolConstant(%s)", bool_ ? "true" : "false");
					case INTEGER:
						return makeString("IntegerConstant(%lld)", integer_);
					case FLOATINGPOINT:
						return makeString("FloatConstant(%Lf)", float_);
					case STRING:
						return makeString("StringConstant(\"%s\")", escapeString(string_).c_str());
					default:
						return "[UNKNOWN CONSTANT]";
				}
			}
			
		private:
			inline Constant(Type type)
				: type_(type){ }
			
			Type type_;
		
			union{
				bool bool_;
				IntegerVal integer_;
				FloatVal float_;
			};
			std::string string_;
		
	};

}

#endif
