#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>
#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		MethodPattern DefaultConstructorPattern() {
			return MethodPattern("Create", IS_STATIC, DO_NOT_CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern NullConstructorPattern() {
			return MethodPattern("Null", IS_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern CastConstructorPattern(const std::string& name) {
			return MethodPattern(name + "_cast", IS_STATIC, CHECK_PARAM_COUNT, 1);
		}
		
		MethodPattern ImplicitCopyPattern() {
			return MethodPattern("implicitCopy", IS_NOT_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern OpAddressPattern() {
			return MethodPattern("address", IS_NOT_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern OpAssignPattern() {
			return MethodPattern("assign", IS_NOT_STATIC, CHECK_PARAM_COUNT, 1);
		}
		
		MethodPattern OpDissolvePattern() {
			return MethodPattern("dissolve", IS_NOT_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern OpMovePattern() {
			return MethodPattern("move", IS_NOT_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		MethodPattern DestructorPattern() {
			return MethodPattern("__destructor", IS_NOT_STATIC, CHECK_PARAM_COUNT, 0);
		}
		
		std::vector<MethodPattern> GetStandardPatterns() {
			std::vector<MethodPattern> patterns;
			patterns.push_back(DefaultConstructorPattern());
			patterns.push_back(NullConstructorPattern());
			patterns.push_back(ImplicitCopyPattern());
			patterns.push_back(OpAddressPattern());
			patterns.push_back(OpAssignPattern());
			patterns.push_back(OpDissolvePattern());
			patterns.push_back(OpMovePattern());
			patterns.push_back(DestructorPattern());
			
			// TODO: Refactor!
			patterns.push_back(CastConstructorPattern("float"));
			patterns.push_back(CastConstructorPattern("double"));
			
			patterns.push_back(CastConstructorPattern("char"));
			patterns.push_back(CastConstructorPattern("short"));
			patterns.push_back(CastConstructorPattern("int"));
			patterns.push_back(CastConstructorPattern("long"));
			patterns.push_back(CastConstructorPattern("longlong"));
			
			patterns.push_back(CastConstructorPattern("uchar"));
			patterns.push_back(CastConstructorPattern("ushort"));
			patterns.push_back(CastConstructorPattern("uint"));
			patterns.push_back(CastConstructorPattern("ulong"));
			patterns.push_back(CastConstructorPattern("ulonglong"));
			
			patterns.push_back(CastConstructorPattern("int8"));
			patterns.push_back(CastConstructorPattern("int16"));
			patterns.push_back(CastConstructorPattern("int32"));
			patterns.push_back(CastConstructorPattern("int64"));
			
			patterns.push_back(CastConstructorPattern("uint8"));
			patterns.push_back(CastConstructorPattern("uint16"));
			patterns.push_back(CastConstructorPattern("uint32"));
			patterns.push_back(CastConstructorPattern("uint64"));
			return patterns;
		}
	
		Node FindMethodPattern(const MethodPattern& pattern, const Node& typeNode) {
			assert(typeNode.isTypeInstance());
			
			const Node functionNode = typeNode.getChild(pattern.name);
			if (!functionNode.isFunction()) return Node::None();
			
			SEM::Function* function = functionNode.getSEMFunction();
			
			if (function->isStaticMethod() != pattern.isStatic) return Node::None();
			
			SEM::Type* type = function->type();
			
			assert(!type->isFunctionVarArg() && "Methods can never be var-arg.");
			
			if (pattern.checkParamCount && type->getFunctionParameterTypes().size() != pattern.numParameters) return Node::None();
			
			return functionNode;
		}
	}
	
}


