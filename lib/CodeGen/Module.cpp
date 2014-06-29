#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>

#include <locic/Debug.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
		
		Module::Module(const std::string& name, const TargetInfo& targetInfo, Debug::Module& pDebugModule)
			: module_(new llvm::Module(name.c_str(), llvm::getGlobalContext())),
			  targetInfo_(targetInfo), abi_(llvm_abi::createABI(module_.get(), targetInfo_.getTargetTriple())),
			  templateRootFunctionMap_(isTypeLessThan),
			  debugBuilder_(*this), debugModule_(pDebugModule) {
			module_->setDataLayout(abi_->dataLayout().getStringRepresentation());
			module_->setTargetTriple(targetInfo_.getTargetTriple());
			
			primitiveMap_.insert(std::make_pair("void_t", PrimitiveVoid));
			primitiveMap_.insert(std::make_pair("null_t", PrimitiveNull));
			primitiveMap_.insert(std::make_pair("bool", PrimitiveBool));
			primitiveMap_.insert(std::make_pair("unichar", PrimitiveUnichar));
			primitiveMap_.insert(std::make_pair("float_t", PrimitiveFloat));
			primitiveMap_.insert(std::make_pair("double_t", PrimitiveDouble));
			primitiveMap_.insert(std::make_pair("longdouble_t", PrimitiveLongDouble));
			primitiveMap_.insert(std::make_pair("__ref", PrimitiveRef));
			primitiveMap_.insert(std::make_pair("__ptr", PrimitivePtr));
			primitiveMap_.insert(std::make_pair("ptr_lval", PrimitivePtrLval));
			primitiveMap_.insert(std::make_pair("value_lval", PrimitiveValueLval));
			primitiveMap_.insert(std::make_pair("member_lval", PrimitiveMemberLval));
			primitiveMap_.insert(std::make_pair("typename_t", PrimitiveTypename));
			
			primitiveMap_.insert(std::make_pair("int8_t", PrimitiveInt8));
			primitiveMap_.insert(std::make_pair("int16_t", PrimitiveInt16));
			primitiveMap_.insert(std::make_pair("int32_t", PrimitiveInt32));
			primitiveMap_.insert(std::make_pair("int64_t", PrimitiveInt64));
			primitiveMap_.insert(std::make_pair("byte_t", PrimitiveByte));
			primitiveMap_.insert(std::make_pair("short_t", PrimitiveShort));
			primitiveMap_.insert(std::make_pair("int_t", PrimitiveInt));
			primitiveMap_.insert(std::make_pair("long_t", PrimitiveLong));
			primitiveMap_.insert(std::make_pair("longlong_t", PrimitiveLongLong));
			primitiveMap_.insert(std::make_pair("ssize_t", PrimitiveSSize));
			
			primitiveMap_.insert(std::make_pair("uint8_t", PrimitiveUInt8));
			primitiveMap_.insert(std::make_pair("uint16_t", PrimitiveUInt16));
			primitiveMap_.insert(std::make_pair("uint32_t", PrimitiveUInt32));
			primitiveMap_.insert(std::make_pair("uint64_t", PrimitiveUInt64));
			primitiveMap_.insert(std::make_pair("ubyte_t", PrimitiveUByte));
			primitiveMap_.insert(std::make_pair("ushort_t", PrimitiveUShort));
			primitiveMap_.insert(std::make_pair("uint_t", PrimitiveUInt));
			primitiveMap_.insert(std::make_pair("ulong_t", PrimitiveULong));
			primitiveMap_.insert(std::make_pair("ulonglong_t", PrimitiveULongLong));
			primitiveMap_.insert(std::make_pair("size_t", PrimitiveSize));
		}
		
		void Module::dump() const {
			module_->dump();
		}
		
		void Module::dumpToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			ostream << *(module_);
		}
		
		void Module::writeBitCodeToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			llvm::WriteBitcodeToFile(module_.get(), ostream);
		}
		
		const TargetInfo& Module::getTargetInfo() const {
			return targetInfo_;
		}
		
		llvm_abi::ABI& Module::abi() {
			return *abi_;
		}
		
		const llvm_abi::ABI& Module::abi() const {
			return *abi_;
		}
		
		llvm_abi::Context& Module::abiContext() {
			return abiContext_;
		}
		
		llvm::LLVMContext& Module::getLLVMContext() const {
			return module_->getContext();
		}
		
		llvm::Module& Module::getLLVMModule() const {
			return *module_;
		}
		
		llvm::Module* Module::getLLVMModulePtr() const {
			return module_.get();
		}
		
		AttributeMap& Module::attributeMap() {
			return attributeMap_;
		}
		
		BitsRequiredGlobalMap& Module::bitsRequiredGlobalMap() {
			return bitsRequiredGlobalMap_;
		}
		
		DestructorMap& Module::getDestructorMap() {
			return destructorMap_;
		}
		
		FunctionMap& Module::getFunctionMap() {
			return functionMap_;
		}
		
		FunctionDeclMap& Module::getFunctionDeclMap() {
			return functionDeclMap_;
		}
		
		FunctionPtrStubMap& Module::functionPtrStubMap() {
			return functionPtrStubMap_;
		}
		
		MemberOffsetFunctionMap& Module::memberOffsetFunctionMap() {
			return memberOffsetFunctionMap_;
		}
		
		MemberVarMap& Module::getMemberVarMap() {
			return memberVarMap_;
		}
		
		const MemberVarMap& Module::getMemberVarMap() const {
			return memberVarMap_;
		}
		
		StandardTypeMap& Module::standardTypeMap() {
			return standardTypeMap_;
		}
		
		TemplateBuilder& Module::typeTemplateBuilder(SEM::TypeInstance* typeInstance) {
			return templateBuilderMap_[typeInstance];
		}
		
		TemplateRootFunctionMap& Module::templateRootFunctionMap() {
			return templateRootFunctionMap_;
		}
		
		TypeMap& Module::getTypeMap() {
			return typeMap_;
		}
		
		const TypeMap& Module::getTypeMap() const {
			return typeMap_;
		}
		
		TypeInstanceMap& Module::typeInstanceMap() {
			return typeInstanceMap_;
		}
		
		llvm::GlobalVariable* Module::createConstGlobal(const std::string& name,
				llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
				llvm::Constant* value) {
			const bool isConstant = true;
			return new llvm::GlobalVariable(getLLVMModule(), type, isConstant, linkage, value, name);
		}
		
		DebugBuilder& Module::debugBuilder() {
			return debugBuilder_;
		}
		
		Debug::Module& Module::debugModule() {
			return debugModule_;
		}
		
		PrimitiveKind Module::primitiveKind(const std::string& name) const {
			return primitiveMap_.at(name);
		}
		
		CompareResult compareTypes(SEM::Type* const first, SEM::Type* const second) {
			if (first == second) {
				return COMPARE_EQUAL;
			}
			
			if (first->kind() != second->kind()) {
				return first->kind() < second->kind() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isConst() != second->isConst()) {
				return first->isConst() && !second->isConst() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isLval() != second->isLval()) {
				return first->isLval() && !second->isLval() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isRef() != second->isRef()) {
				return first->isRef() && !second->isRef() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isLval()) {
				const auto result = compareTypes(first->lvalTarget(), second->lvalTarget());
				if (result != COMPARE_EQUAL) {
					return result;
				}
			}
			
			if (first->isRef()) {
				const auto result = compareTypes(first->refTarget(), second->refTarget());
				if (result != COMPARE_EQUAL) {
					return result;
				}
			}
			
			switch (first->kind()) {
				case SEM::Type::OBJECT: {
					if (first->getObjectType() != second->getObjectType()) {
						return first->getObjectType() < second->getObjectType() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->templateArguments().size() != second->templateArguments().size()) {
						return first->templateArguments().size() < second->templateArguments().size() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					for (size_t i = 0; i < first->templateArguments().size(); i++) {
						const auto result = compareTypes(first->templateArguments().at(i), second->templateArguments().at(i));
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					return COMPARE_EQUAL;
				}
				
				case SEM::Type::FUNCTION: {
					if (first->isFunctionVarArg() != second->isFunctionVarArg()) {
						return first->isFunctionVarArg() && !second->isFunctionVarArg() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionMethod() != second->isFunctionMethod()) {
						return first->isFunctionMethod() && !second->isFunctionMethod() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionTemplatedMethod() != second->isFunctionTemplatedMethod()) {
						return first->isFunctionTemplatedMethod() && !second->isFunctionTemplatedMethod() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionNoExcept() != second->isFunctionNoExcept()) {
						return first->isFunctionNoExcept() && !second->isFunctionNoExcept() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					{
						const auto result = compareTypes(first->getFunctionReturnType(), second->getFunctionReturnType());
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					const auto& firstList = first->getFunctionParameterTypes();
					const auto& secondList = second->getFunctionParameterTypes();
					
					if (firstList.size() != secondList.size()) {
						return firstList.size() < secondList.size() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					for (size_t i = 0; i < firstList.size(); i++) {
						const auto result = compareTypes(firstList.at(i), secondList.at(i));
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					return COMPARE_EQUAL;
				}
				
				case SEM::Type::METHOD: {
					return compareTypes(first->getMethodFunctionType(), second->getMethodFunctionType());
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return compareTypes(first->getInterfaceMethodFunctionType(), second->getInterfaceMethodFunctionType());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					if (first->getTemplateVar() != second->getTemplateVar()) {
						return first->getTemplateVar() < second->getTemplateVar() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					return COMPARE_EQUAL;
				}
				
				default: {
					llvm_unreachable("Unknown type enum in comparison.");
				}
			}
		}
		
	}
	
}

