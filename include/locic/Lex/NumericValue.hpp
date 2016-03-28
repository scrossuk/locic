#ifndef LOCIC_LEX_NUMERICVALUE_HPP
#define LOCIC_LEX_NUMERICVALUE_HPP

#include <cassert>

#include <locic/Support/APInt.hpp>

namespace locic {
	
	namespace Lex {
		
		class NumericValue {
		public:
			enum Kind {
				INTEGER,
				FLOAT,
				VERSION
			};
			
			static NumericValue Integer(APInt integerValue) {
				NumericValue value(INTEGER);
				value.integerValue_ = std::move(integerValue);
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
			
			APInt& integerValue() {
				assert(isInteger());
				return integerValue_;
			}
			
			const APInt& integerValue() const {
				assert(isInteger());
				return integerValue_;
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
			APInt integerValue_;
			
			union {
				long double floatValue;
				class Version versionValue;
			} data_;
			
		};
		
	}
	
}

#endif