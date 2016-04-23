#ifndef LOCIC_FRONTEND_DIAGNOSTICRENDERER_HPP
#define LOCIC_FRONTEND_DIAGNOSTICRENDERER_HPP

#include <ostream>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation;
		
	}
	
	class Diag;
	enum class DiagLevel;
	class OptionalDiag;
	
	class DiagnosticRenderer {
	public:
		DiagnosticRenderer(bool useColors);
		
		void setColor(std::ostream& stream, const char* color);
		
		void resetColor(std::ostream& stream);
		
		void emitDiagnostic(std::ostream& stream, const Diag& diagnostic,
		                    const Debug::SourceLocation& location,
		                    const OptionalDiag& chain);
		
		void emitChainDiagnostic(std::ostream& stream, const Diag& diagnostic,
		                         const Debug::SourceLocation& location);
		
		void emitDiagnosticMessage(std::ostream& stream, const Diag& diagnostic,
		                           const Debug::SourceLocation& location, bool isChain);
		
		void emitDiagnosticLevel(std::ostream& stream, DiagLevel level);
		
		void emitCodeContext(std::ostream& stream, const Debug::SourceLocation& location);
		
		size_t findLineStart(FILE* file, size_t begin);
		
		size_t findLineEnd(FILE* file, size_t end);
		
		void emitDiagnosticSummary(std::ostream& stream);
		
	private:
		bool useColors_;
		size_t numWarnings_, numErrors_;
		
	};
	
}

#endif
