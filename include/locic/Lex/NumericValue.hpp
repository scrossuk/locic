#ifndef LOCIC_LEX_NUMERICVALUE_HPP
#define LOCIC_LEX_NUMERICVALUE_HPP

#include <cassert>

namespace locic {
	
	namespace Lex {
		
		class NumericValue {
		public:
			enum Kind {
				INTEGER,
				FLOAT,
				VERSION
			};
			
			static NumericValue Integer(unsigned long long integerValue) {
				NumericValue value(INTEGER);
				value.data_.integerValue = integerValue;
				return value;
			}
			
			static NumericValue Float(long double floatValue) {
				NumericValue value(FLOAT);
				value.data_.floatValue = floatValue;
				return value;
			}
			
			static NumericValue Version(const Version versionValue) {
				NumericValue value(VERSION);
				value.data_.versionValue = versionValue;
				return value;
			}
			
			Kind kind() const {
				return kind_;
			}
			
			bool isInteger() const {
				return kind() == INTEGER;
			}
			
			unsigned long long integerValue() const {
				assert(isInteger());
				return data_.integerValue;
			}
			
			bool isFloat() const {
				return kind() == FLOAT;
			}
			
			double floatValue() const {
				assert(isFloat());
				return data_.floatValue;
			}
			
			bool isVersion() const {
				return kind() == VERSION;
			}
			
			class Version versionValue() const {
				assert(isVersion());
				return data_.versionValue;
			}
			
		private:
			NumericValue(Kind argKind)
			: kind_(argKind) { }
			
			Kind kind_;
			
			union {
				unsigned long long integerValue;
				long double floatValue;
				class Version versionValue;
			} data_;
			
		};
		
	}
	
}

#endif