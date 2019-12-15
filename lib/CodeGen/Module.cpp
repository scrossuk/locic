#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/Debug.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/PrimitiveID.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/VirtualCall/GenericVirtualCallABI.hpp>

namespace locic {

	namespace CodeGen {
		
		Module::Module(InternalContext& argContext, const std::string& name, Debug::Module& pDebugModule, const BuildOptions& pBuildOptions)
			: context_(argContext), module_(new llvm::Module(name.c_str(), context_.llvmContext())),
			  abi_(llvm_abi::createABI(*module_, context_.targetTriple())),
			  virtualCallABI_(new GenericVirtualCallABI(*this)),
			  debugBuilder_(*this), debugModule_(pDebugModule), buildOptions_(pBuildOptions),
			  astFunctionGenerator_(*this) {
			
			module_->setDataLayout(context_.dataLayout().getStringRepresentation());
			module_->setTargetTriple(context_.targetTriple().getTriple());
			module_->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
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
			module_->print(llvm::dbgs(), nullptr, /*ShouldPreserveUseListOrder=*/false, /*IsForDebug=*/true);
		}
		
		void Module::dumpToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			ostream << *(module_);
		}
		
		void Module::writeBitCodeToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			llvm::WriteBitcodeToFile(*module_, ostream);
		}
		
		llvm_abi::ABI& Module::abi() {
			return *abi_;
		}
		
		const llvm_abi::ABI& Module::abi() const {
			return *abi_;
		}
		
		const llvm_abi::TypeBuilder& Module::abiTypeBuilder() const {
			return abi().typeInfo().typeBuilder();
		}
		
		llvm::Type*
		Module::getLLVMType(const llvm_abi::Type type) const {
			return abi().typeInfo().getLLVMType(type);
		}
		
		VirtualCallABI& Module::virtualCallABI() {
			return *virtualCallABI_;
		}
		
		llvm::LLVMContext& Module::getLLVMContext() const {
			return context_.llvmContext();
		}
		
		std::unique_ptr<llvm::Module> Module::releaseLLVMModule() {
			auto releasedValue = std::move(module_);
			module_ = std::unique_ptr<llvm::Module>();
			return releasedValue;
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
		
		MoveFunctionMap& Module::getMoveFunctionMap() {
			return moveFunctionMap_;
		}
		
		const Primitive& Module::getPrimitive(const AST::TypeInstance& typeInstance) const {
			return primitiveMap_.getPrimitive(typeInstance);
		}
		
		StandardTypeMap& Module::standardTypeMap() {
			return standardTypeMap_;
		}
		
		TemplateBuilder& Module::templateBuilder(TemplatedObject templatedObject) {
			auto insertPair = std::make_pair(templatedObject,
			                                 TemplateBuilder(templatedObject));
			return templateBuilderMap_.insert(std::move(insertPair)).first->second;
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
		
		PrimitiveID Module::primitiveID(const String& name) const {
			return context_.getPrimitiveID(name);
		}
		
		ASTFunctionGenerator& Module::astFunctionGenerator() {
			return astFunctionGenerator_;
		}
		
		void Module::verify() const {
			// Only verify modules when built in debug mode.
#if !defined(NDEBUG)
			const bool result = llvm::verifyModule(*module_, &(llvm::dbgs()));
			if (result) {
				dump();
				throw std::runtime_error("Verification failed for module.");
			}
#endif
		}
		
	}
	
}

