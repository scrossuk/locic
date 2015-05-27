#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>

#include <locic/Debug.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		Module::Module(InternalContext& argContext, const std::string& name, Debug::Module& pDebugModule, const BuildOptions& pBuildOptions)
			: context_(argContext), module_(new llvm::Module(name.c_str(), context_.llvmContext())),
			  abi_(llvm_abi::createABI(module_.get(), context_.targetTriple())),
			  debugBuilder_(*this), debugModule_(pDebugModule), buildOptions_(pBuildOptions) {
			
			module_->setDataLayout(context_.dataLayout().getStringRepresentation());
			module_->setTargetTriple(context_.targetTriple().getTriple());
			
#if LOCIC_LLVM_VERSION >= 304
			module_->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
#endif
			
			primitiveMap_.insert(std::make_pair(getCString("void_t"), PrimitiveVoid));
			primitiveMap_.insert(std::make_pair(getCString("null_t"), PrimitiveNull));
			primitiveMap_.insert(std::make_pair(getCString("bool"), PrimitiveBool));
			primitiveMap_.insert(std::make_pair(getCString("float_t"), PrimitiveFloat));
			primitiveMap_.insert(std::make_pair(getCString("double_t"), PrimitiveDouble));
			primitiveMap_.insert(std::make_pair(getCString("longdouble_t"), PrimitiveLongDouble));
			primitiveMap_.insert(std::make_pair(getCString("__ref"), PrimitiveRef));
			primitiveMap_.insert(std::make_pair(getCString("__ptr"), PrimitivePtr));
			primitiveMap_.insert(std::make_pair(getCString("ptr_lval"), PrimitivePtrLval));
			primitiveMap_.insert(std::make_pair(getCString("value_lval"), PrimitiveValueLval));
			primitiveMap_.insert(std::make_pair(getCString("final_lval"), PrimitiveFinalLval));
			primitiveMap_.insert(std::make_pair(getCString("typename_t"), PrimitiveTypename));
			primitiveMap_.insert(std::make_pair(getCString("compare_result_t"), PrimitiveCompareResult));
			
			primitiveMap_.insert(std::make_pair(getCString("function0_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function1_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function2_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function3_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function4_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function5_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function6_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function7_ptr_t"), PrimitiveFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("function8_ptr_t"), PrimitiveFunctionPtr));
			
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod0_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod1_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod2_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod3_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod4_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod5_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod6_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod7_t"), PrimitiveInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("interfacemethod8_t"), PrimitiveInterfaceMethod));
			
			primitiveMap_.insert(std::make_pair(getCString("method0_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method1_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method2_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method3_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method4_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method5_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method6_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method7_t"), PrimitiveMethod));
			primitiveMap_.insert(std::make_pair(getCString("method8_t"), PrimitiveMethod));
			
			primitiveMap_.insert(std::make_pair(getCString("methodfunction0_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction1_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction2_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction3_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction4_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction5_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction6_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction7_ptr_t"), PrimitiveMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("methodfunction8_ptr_t"), PrimitiveMethodFunctionPtr));
			
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod0_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod1_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod2_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod3_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod4_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod5_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod6_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod7_t"), PrimitiveStaticInterfaceMethod));
			primitiveMap_.insert(std::make_pair(getCString("staticinterfacemethod8_t"), PrimitiveStaticInterfaceMethod));
			
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction0_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction1_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction2_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction3_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction4_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction5_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction6_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction7_ptr_t"), PrimitiveTemplatedFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedfunction8_ptr_t"), PrimitiveTemplatedFunctionPtr));
			
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod0_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod1_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod2_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod3_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod4_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod5_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod6_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod7_t"), PrimitiveTemplatedMethod));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethod8_t"), PrimitiveTemplatedMethod));
			
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction0_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction1_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction2_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction3_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction4_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction5_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction6_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction7_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("templatedmethodfunction8_ptr_t"), PrimitiveTemplatedMethodFunctionPtr));
			
			primitiveMap_.insert(std::make_pair(getCString("varargfunction0_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction1_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction2_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction3_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction4_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction5_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction6_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction7_ptr_t"), PrimitiveVarArgFunctionPtr));
			primitiveMap_.insert(std::make_pair(getCString("varargfunction8_ptr_t"), PrimitiveVarArgFunctionPtr));
			
			primitiveMap_.insert(std::make_pair(getCString("int8_t"), PrimitiveInt8));
			primitiveMap_.insert(std::make_pair(getCString("int16_t"), PrimitiveInt16));
			primitiveMap_.insert(std::make_pair(getCString("int32_t"), PrimitiveInt32));
			primitiveMap_.insert(std::make_pair(getCString("int64_t"), PrimitiveInt64));
			primitiveMap_.insert(std::make_pair(getCString("byte_t"), PrimitiveByte));
			primitiveMap_.insert(std::make_pair(getCString("short_t"), PrimitiveShort));
			primitiveMap_.insert(std::make_pair(getCString("int_t"), PrimitiveInt));
			primitiveMap_.insert(std::make_pair(getCString("long_t"), PrimitiveLong));
			primitiveMap_.insert(std::make_pair(getCString("longlong_t"), PrimitiveLongLong));
			primitiveMap_.insert(std::make_pair(getCString("ssize_t"), PrimitiveSSize));
			primitiveMap_.insert(std::make_pair(getCString("ptrdiff_t"), PrimitivePtrDiff));
			
			primitiveMap_.insert(std::make_pair(getCString("uint8_t"), PrimitiveUInt8));
			primitiveMap_.insert(std::make_pair(getCString("uint16_t"), PrimitiveUInt16));
			primitiveMap_.insert(std::make_pair(getCString("uint32_t"), PrimitiveUInt32));
			primitiveMap_.insert(std::make_pair(getCString("uint64_t"), PrimitiveUInt64));
			primitiveMap_.insert(std::make_pair(getCString("ubyte_t"), PrimitiveUByte));
			primitiveMap_.insert(std::make_pair(getCString("ushort_t"), PrimitiveUShort));
			primitiveMap_.insert(std::make_pair(getCString("uint_t"), PrimitiveUInt));
			primitiveMap_.insert(std::make_pair(getCString("ulong_t"), PrimitiveULong));
			primitiveMap_.insert(std::make_pair(getCString("ulonglong_t"), PrimitiveULongLong));
			primitiveMap_.insert(std::make_pair(getCString("size_t"), PrimitiveSize));
		}
		
		InternalContext& Module::context() {
			return context_;
		}
		
		String Module::getCString(const char* const cString) const {
			return String(context_.stringHost(), cString);
		}
		
		String Module::getString(std::string stringValue) const {
			return String(context_.stringHost(), std::move(stringValue));
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
		
		llvm_abi::ABI& Module::abi() {
			return *abi_;
		}
		
		const llvm_abi::ABI& Module::abi() const {
			return *abi_;
		}
		
		llvm_abi::Context& Module::abiContext() {
			return context_.llvmABIContext();
		}
		
		llvm::LLVMContext& Module::getLLVMContext() const {
			return context_.llvmContext();
		}
		
		std::unique_ptr<llvm::Module> Module::releaseLLVMModule() {
			auto releasedValue = std::move(module_);
			module_ = std::unique_ptr<llvm::Module>();
			return std::move(releasedValue);
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
		
		MangledNameMap& Module::mangledNameMap() {
			return mangledNameMap_;
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
		
		MoveFunctionMap& Module::getMoveFunctionMap() {
			return moveFunctionMap_;
		}
		
		StandardTypeMap& Module::standardTypeMap() {
			return standardTypeMap_;
		}
		
		TemplateBuilder& Module::templateBuilder(TemplatedObject templatedObject) {
			return templateBuilderMap_[templatedObject];
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
		
		llvm::GlobalVariable* Module::createConstGlobal(const String& name,
				llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
				llvm::Constant* value) {
			const bool isConstant = true;
			return new llvm::GlobalVariable(getLLVMModule(), type, isConstant, linkage, value, name.c_str());
		}
		
		DebugBuilder& Module::debugBuilder() {
			return debugBuilder_;
		}
		
		Debug::Module& Module::debugModule() {
			return debugModule_;
		}
		
		const BuildOptions& Module::buildOptions() const {
			return buildOptions_;
		}
		
		PrimitiveKind Module::primitiveKind(const String& name) const {
			const auto iterator = primitiveMap_.find(name);
			if (iterator == primitiveMap_.end()) {
				printf("%s\n", name.c_str());
				llvm_unreachable("Failed to find primitive type.");
			}
			return iterator->second;
		}
		
		void Module::verify() const {
			// Only verify modules when built in debug mode.
#if !defined(NDEBUG)
#if LOCIC_LLVM_VERSION >= 305
			llvm::raw_os_ostream cerrStream(std::cerr);
			const bool result = llvm::verifyModule(*module_, &cerrStream);
			if (result)
			{
				//throw std::runtime_error("Verification failed for module.");
			}
#else
			(void) llvm::verifyModule(module_, llvm::AbortProcessAction);
#endif
#endif
		}
		
	}
	
}

