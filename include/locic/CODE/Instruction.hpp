#ifndef LOCIC_CODE_INSTRUCTION_HPP
#define LOCIC_CODE_INSTRUCTION_HPP

#include <locic/Constant.hpp>
#include <locic/CODE/Type.hpp>

namespace locic {

	namespace CODE {
		
		struct Global {
			Name name;
			Type * type;
			
			inline Global()
				: type(NULL){ }
		};
		
		enum BinaryOp{
			ADD,
			SUB,
			
		};
	
		struct Instruction {
			enum TypeEnum {
				CONSTANT,
				MOVE,
				JUMP,
				CONDJUMP,
				GETELEMENT,
				GLOBALREF,
				CALL,
				VIRTUALCALL
			} typeEnum;
			
			Constant* constant;
			Global* global;
			Var* sourceVar;
			Var* operandVar;
			Var* destVar;
			size_t position;
			
			inline Instruction(TypeEnum t)
				: typeEnum(t),
				global(NULL),
				constant(NULL),
				sourceVar(NULL),
				destVar(NULL),
				position(0){ }
			
			inline static Instruction Constant(Constant * constant, Var * destVar){
				Instruction instruction(CONSTANT);
				instruction.constant = constant;
				instruction.destVar = destVar;
				return instruction;
			}
			
			inline static Instruction Move(Var * sourceVar, Var * destVar){
				Instruction instruction(MOVE);
				instruction.sourceVar = sourceVar;
				instruction.destVar = destVar;
				return instruction;
			}
			
			inline static Instruction Jump(size_t position){
				Instruction instruction(JUMP);
				instruction.position = position;
				return instruction;
			}
			
			inline static Instruction ConditionalJump(Var * condition, size_t position){
				Instruction instruction(CONDJUMP);
				instruction.sourceVar = condition;
				instruction.position = position;
				return instruction;
			}
			
			inline static Instruction GetElement(Var * sourceVar, size_t elementOffset, Var * destVar){
				Instruction instruction(GETELEMENT);
				instruction.sourceVar = sourceVar;
				instruction.destVar = destVar;
				instruction.position = elementOffset;
				return instruction;
			}
			
			inline static Instruction GlobalRef(Global * global, Var * destVar){
				Instruction instruction(GLOBALREF);
				instruction.global = global;
				instruction.destVar = destVar;
				return instruction;
			}
			
			inline static Instruction GEP(Var * sourceVar, size_t elementOffset, Var * destVar){
				Instruction instruction(GEP);
				instruction.sourceVar = sourceVar;
				instruction.destVar = destVar;
				instruction.position = elementOffset;
				return instruction;
			}
		};
		
		var S * v0;
		v0 = 
		store 
		
		gep args 0 arg0
		move arg0 local0
		constant 0 local1
		add local0 local1 local2
		
		
		
	}
	
}

#endif
