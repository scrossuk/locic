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
			assert(iterator != primitiveMap_.end() && "Failed to find primitive type!");
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

