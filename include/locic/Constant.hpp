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
			
			inline Kind kind() const{
				return kind_;
			}
			
			inline bool boolValue() const{
				assert(kind_ == BOOLEAN);
				return bool_;
			}
			
			inline IntegerVal integerValue() const{
				assert(kind_ == INTEGER);
				return integer_;
			}
			
			inline FloatVal floatValue() const{
				assert(kind_ == FLOATINGPOINT);
				return float_;
			}
			
			inline const std::string& stringValue() const{
				assert(kind_ == STRING);
				return string_;
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
					case STRING:
						return makeString("StringConstant(\"%s\")", escapeString(stringValue()).c_str());
					default:
						return "[UNKNOWN CONSTANT]";
				}
			}
			
		private:
			inline Constant(Kind pKind)
				: kind_(pKind) { }
			
			Kind kind_;
			
			union{
				bool bool_;
				IntegerVal integer_;
				FloatVal float_;
			};
			
			std::string string_;
		
	};

}

#endif
