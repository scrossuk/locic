#include <iostream>
#include <stdexcept>

#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/TargetOptions.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/MethodID.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/SharedMaps.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Triple parseTargetTripleString(const std::string& targetTripleString) {
			if (!targetTripleString.empty()) {
				return llvm::Triple(llvm::Triple::normalize(targetTripleString));
			} else {
				// Default to host architecture.
				return llvm::Triple(llvm::sys::getDefaultTargetTriple());
			}
		}
		
		std::string getX86TargetCPU(const llvm::Triple& triple) {
			// Select the default CPU if none was given (or detection failed).
			// Intel Macs are relatively recent, take advantage of that.
			if (triple.isOSDarwin()) {
				return triple.isArch64Bit() ? "core2" : "yonah";
			}
			// Everything else goes to x86-64 in 64-bit mode.
			if (triple.isArch64Bit()) {
				return "x86-64";
			}
			if (triple.getOSName().startswith("haiku")) {
				return "i586";
			}
			if (triple.getOSName().startswith("openbsd")) {
				return "i486";
			}
			if (triple.getOSName().startswith("bitrig")) {
				return "i686";
			}
			if (triple.getOSName().startswith("freebsd")) {
				return "i486";
			}
			if (triple.getOSName().startswith("netbsd")) {
				return "i486";
			}
			// All x86 devices running Android have core2 as their common
			// denominator. This makes a better choice than pentium4.
			if (triple.getEnvironment() == llvm::Triple::Android) {
				return "core2";
			}
			// Fallback to p4.
			return "pentium4";
		}
		
		std::string getARMTargetCPU(const llvm::Triple& triple)
		{
			const char *result = llvm::StringSwitch<const char *>(triple.getArchName())
			.Cases("armv2", "armv2a","arm2")
			.Case("armv3", "arm6")
			.Case("armv3m", "arm7m")
			.Case("armv4", "strongarm")
			.Case("armv4t", "arm7tdmi")
			.Cases("armv5", "armv5t", "arm10tdmi")
			.Cases("armv5e", "armv5te", "arm1026ejs")
			.Case("armv5tej", "arm926ej-s")
			.Cases("armv6", "armv6k", "arm1136jf-s")
			.Case("armv6j", "arm1136j-s")
			.Cases("armv6z", "armv6zk", "arm1176jzf-s")
			.Case("armv6t2", "arm1156t2-s")
			.Cases("armv6m", "armv6-m", "cortex-m0")
			.Cases("armv7", "armv7a", "armv7-a", "cortex-a8")
			.Cases("armv7l", "armv7-l", "cortex-a8")
			.Cases("armv7f", "armv7-f", "cortex-a9-mp")
			.Cases("armv7s", "armv7-s", "swift")
			.Cases("armv7r", "armv7-r", "cortex-r4")
			.Cases("armv7m", "armv7-m", "cortex-m3")
			.Cases("armv7em", "armv7e-m", "cortex-m4")
			.Cases("armv8", "armv8a", "armv8-a", "cortex-a53")
			.Case("ep9312", "ep9312")
			.Case("iwmmxt", "iwmmxt")
			.Case("xscale", "xscale")
			// If all else failed, return the most base CPU with thumb interworking
			// supported by LLVM.
			.Default(0);
			if (result != nullptr) {
				return result;
			}
			return (triple.getEnvironment() == llvm::Triple::GNUEABIHF) ?
				"arm1176jzf-s" : "arm7tdmi";
		}
		
		std::string getTargetCPU(const std::string& specifiedCPU, const llvm::Triple& triple) {
			if (!specifiedCPU.empty()) {
				if (specifiedCPU == "native") {
					// Try to get the host CPU name.
					const std::string hostCPU = llvm::sys::getHostCPUName();
					if (!hostCPU.empty() && hostCPU != "generic") {
						return hostCPU;
					}
				} else {
					return specifiedCPU;
				}
			}
			switch (triple.getArch())
			{
				case llvm::Triple::x86:
				case llvm::Triple::x86_64:
					return getX86TargetCPU(triple);
				case llvm::Triple::arm:
					return getARMTargetCPU(triple);
				default:
					// Unknown platform, so just pass to LLVM and let it decide.
					return specifiedCPU;
			}
		}
		
		InternalContext::InternalContext(const AST::Context& argASTContext,
		                                 const SharedMaps& argSharedMaps,
		                                 const TargetOptions& targetOptions)
		: astContext_(argASTContext), sharedMaps_(argSharedMaps),
		targetTriple_(parseTargetTripleString(targetOptions.triple)),
		target_(nullptr) {
			llvm::InitializeAllTargetInfos();
			llvm::InitializeAllTargets();
			llvm::InitializeAllTargetMCs();
			llvm::InitializeAllAsmPrinters();
			llvm::InitializeAllAsmParsers();
			
			std::string errorString;
			target_ = llvm::TargetRegistry::lookupTarget(targetOptions.arch, targetTriple_, errorString);
			if (target_ == nullptr) {
				llvm::raw_os_ostream out(std::cout);
				llvm::TargetRegistry::printRegisteredTargetsForVersion(out);
				throw std::runtime_error(makeString("Failed to find target for triple '%s'.", targetTriple_.getTriple().c_str()));
			}
			
			llvm::SubtargetFeatures features;
			features.getDefaultSubtargetFeatures(targetTriple_);
			
			// TODO: make these options configurable.
			llvm::TargetOptions llvmTargetOptions;
			
#if LOCIC_LLVM_VERSION >= 309
			llvm::Optional<llvm::Reloc::Model> relocModel;
#else
			llvm::Reloc::Model relocModel = llvm::Reloc::Default;
#endif
			llvm::Optional<llvm::CodeModel::Model> codeModel;
			llvm::CodeGenOpt::Level optimisationLevel = llvm::CodeGenOpt::Default;
			
			targetMachine_ = std::unique_ptr<llvm::TargetMachine>(
				target_->createTargetMachine(
					targetTriple_.getTriple(),
					getTargetCPU(targetOptions.cpu, targetTriple_),
					features.getString(),
					llvmTargetOptions,
					relocModel,
					codeModel,
					optimisationLevel));
			assert(targetMachine_.get() != nullptr);
		}
		
		InternalContext::~InternalContext() { }
		
		const StringHost& InternalContext::stringHost() const {
			return sharedMaps_.stringHost();
		}
		
		MethodID InternalContext::getMethodID(const String& name) const {
			return sharedMaps_.methodIDMap().getMethodID(name);
		}
		
		PrimitiveID InternalContext::getPrimitiveID(const String& name) const {
			return sharedMaps_.primitiveIDMap().getPrimitiveID(name);
		}
		
		const AST::Context& InternalContext::astContext() const {
			return astContext_;
		}
		
		llvm::LLVMContext& InternalContext::llvmContext() {
			return llvmContext_;
		}
		
		const llvm::Triple& InternalContext::targetTriple() const {
			return targetTriple_;
		}
		
		const llvm::Target* InternalContext::target() const {
			return target_;
		}
		
		const llvm::TargetMachine& InternalContext::targetMachine() const {
			return *targetMachine_;
		}
		
		llvm::DataLayout InternalContext::dataLayout() const {
#if LOCIC_LLVM_VERSION >= 308
			return targetMachine().createDataLayout();
#elif LOCIC_LLVM_VERSION >= 307
			return *(targetMachine().getDataLayout());
#elif LOCIC_LLVM_VERSION >= 306
			return *(targetMachine().getSubtargetImpl()->getDataLayout());
#else
			return *(targetMachine().getDataLayout());
#endif
		}
		
	}
	
}

