#ifndef LOCIC_CODEGEN_LIVENESSINDICATOR_HPP
#define LOCIC_CODEGEN_LIVENESSINDICATOR_HPP

namespace locic {
	
	namespace SEM {
		
		class Var;
		
	}
	
	namespace CodeGen {
		
		/**
		 * \brief Liveness Indicator
		 * 
		 * A liveness indicator is the mechanism that determines
		 * whether an object is in a 'live' state. The importance
		 * of this is that destructors and move methods are only
		 * run on objects that are in a 'live' state.
		 */
		class LivenessIndicator {
		public:
			/**
			 * \brief None.
			 * 
			 * No liveness indicator is required.
			 */
			static LivenessIndicator None();
			
			/**
			 * \brief Use a member variable's invalid state.
			 * 
			 * A member variable could have a state which is invalid
			 * (for example, references can't be null) and so the
			 * object can simply use the member's invalid state to
			 * indicate that the object is 'dead'.
			 */
			static LivenessIndicator MemberInvalidState(const SEM::Var& memberVar);
			
			/**
			 * \brief Use custom '__dead' and '__islive' methods.
			 * 
			 * The user can specify methods manually, which helps
			 * in situations where the compiler might otherwise
			 * have to add a prefix byte.
			 */
			static LivenessIndicator CustomMethods();
			
			/**
			 * \brief Use a prefix byte.
			 * 
			 * This is the worst case (and should be rare), since the
			 * object must contain an extra byte to indicate liveness.
			 */
			static LivenessIndicator PrefixByte();
			
			/**
			 * \brief Use a gap byte.
			 * 
			 * Alignment causes padding to be created, which can create
			 * gaps in the middle of an object; these can be used
			 * to indicate whether the object is valid.
			 */
			static LivenessIndicator GapByte(size_t offset);
			
			enum Kind {
				NONE,
				MEMBER_INVALID_STATE,
				CUSTOM_METHODS,
				PREFIX_BYTE,
				GAP_BYTE
			};
			
			Kind kind() const;
			
			bool isNone() const;
			
			bool isMemberInvalidState() const;
			
			const SEM::Var& memberVar() const;
			
			bool isCustomMethods() const;
			
			bool isPrefixByte() const;
			
			bool isGapByte() const;
			
			size_t gapByteOffset() const;
			
		private:
			LivenessIndicator(Kind kind);
			
			Kind kind_;
			
			union {
				const SEM::Var* memberVar;
				size_t offset;
			} data_;
			
		};
		
	}
	
}

#endif
