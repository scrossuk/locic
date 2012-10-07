#ifndef LOCIC_CONSTANT_HPP
#define LOCIC_CONSTANT_HPP

#include <stdint.h>
#include <cassert>
#include <boost/optional.hpp>

namespace Locic{

	class Constant{
		public:
			enum Type{
				NULLVAL,
				BOOLEAN,
				SIGNEDINT,
				UNSIGNEDINT,
				FLOATINGPOINT,
				STRING
			};
			
			enum IntType{
				CHAR,
				INT,
				LONG,
				LONGLONG,
				INT8,
				INT16,
				INT32,
				INT64,
				INT128
			};
			
			enum UintType{
				UCHAR,
				UINT,
				ULONG,
				ULONGLONG,
				UINT8,
				UINT16,
				UINT32,
				UINT64,
				UINT128
			};
			
			enum FloatType{
				FLOAT,
				DOUBLE,
				LONGDOUBLE
			};
	
			enum StringType{
				CSTRING,
				LOCISTRING
			};
			
			typedef long long IntVal;
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
			
			static inline Constant * SignedInt(IntType type, IntVal value){
				Constant * constant = new Constant(SIGNEDINT);
				constant->intType_ = type;
				constant->int_ = value;
				return constant;
			}
			
			static inline Constant * UnsignedInt(UintType type, UintVal value){
				Constant * constant = new Constant(UNSIGNEDINT);
				constant->uintType_ = type;
				constant->uint_ = value;
				return constant;
			}
			
			static inline Constant * Float(FloatType type, FloatVal value){
				Constant * constant = new Constant(FLOATINGPOINT);
				constant->floatType_ = type;
				constant->float_ = value;
				return constant;
			}
			
			static inline Constant * String(StringType type, const std::string& value){
				Constant * constant = new Constant(STRING);
				constant->stringType_ = type;
				constant->string_ = value;
				return constant;
			}
			
			inline Type getType() const{
				return type_;
			}
			
			inline FloatType getFloatType() const{
				assert(type_ == FLOATINGPOINT);
				return floatType_;
			}
			
			inline StringType getStringType() const{
				assert(type_ == STRING);
				return stringType_;
			}
			
			inline bool getBool() const{
				assert(type_ == BOOLEAN);
				return bool_;
			}
			
			inline IntVal getInt() const{
				assert(type_ == SIGNEDINT);
				return int_;
			}
			
			inline UintVal getUint() const{
				assert(type_ == UNSIGNEDINT);
				return uint_;
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
					case SIGNEDINT:
						switch(intType_){
							case CHAR:
								return "char";
							case INT:
								return "int";
							case LONG:
								return "long";
							case LONGLONG:
								return "longlong";
							case INT8:
								return "int8_t";
							case INT16:
								return "int16_t";
							case INT32:
								return "int32_t";
							case INT64:
								return "int64_t";
							case INT128:
								return "int128_t";
							default:
								assert(false && "Unknown signed int constant type");
								return "";
						}
					case UNSIGNEDINT:
						switch(uintType_){
							case UCHAR:
								return "uchar";
							case UINT:
								return "uint";
							case ULONG:
								return "ulong";
							case ULONGLONG:
								return "ulonglong";
							case UINT8:
								return "uint8_t";
							case UINT16:
								return "uint16_t";
							case UINT32:
								return "uint32_t";
							case UINT64:
								return "uint64_t";
							case UINT128:
								return "uint128_t";
							default:
								assert(false && "Unknown unsigned int constant type");
								return "";
						}
					case FLOATINGPOINT:
						switch(floatType_){
							case FLOAT:
								return "float";
							case DOUBLE:
								return "double";
							case LONGDOUBLE:
								return "longdouble";
							default:
								assert(false && "Unknown float constant type");
								return "";
						}
					case STRING:
						switch(floatType_){
							case CSTRING:
								assert(false && "C-String doesn't have a type name");
								return "";
							case LOCISTRING:
								return "string";
							default:
								assert(false && "Unknown string constant type");
								return "";
						}
					default:
						assert(false && "Unknown constant type");
						return "";
				}
			}
			
		private:
			Constant(Type type)
				: type_(type){ }
			
			Type type_;
			
			union{
				IntType intType_;
				UintType uintType_;
				FloatType floatType_;
				StringType stringType_;
			};
		
			union{
				bool bool_;
				IntVal int_;
				UintVal uint_;
				FloatVal float_;
			};
			std::string string_;
		
	};

}

#endif
