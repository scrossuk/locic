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
			
			static NumericValue Version(unsigned long long major,
			                            unsigned long long minor,
			                            unsigned long long build) {
				NumericValue value(VERSION);
				value.data_.versionValue.major = major;
				value.data_.versionValue.minor = minor;
				value.data_.versionValue.build = build;
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
			
		private:
			NumericValue(Kind argKind)
			: kind_(argKind) { }
			
			Kind kind_;
			
			union {
				unsigned long long integerValue;
				long double floatValue;
				struct {
					unsigned long long major;
					unsigned long long minor;
					unsigned long long build;
				} versionValue;
			} data_;
			
		};
		
	}
	
}

#endif