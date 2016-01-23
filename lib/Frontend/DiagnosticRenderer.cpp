#include <ostream>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/DiagnosticRenderer.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Support/ErrorHandling.hpp>

#define MESSAGE_COLOR "\033[0;1;37m"
#define CARET_COLOR "\033[0;1;32m"
#define ERROR_COLOR "\033[0;1;31m"
#define WARNING_COLOR "\033[0;1;35m"
#define NOTE_COLOR "\033[0;1;30m"
#define RESET_COLOR "\x1B[0m"

namespace locic {
	
	DiagnosticRenderer::DiagnosticRenderer(const bool useColors)
	: useColors_(useColors), numWarnings_(0), numErrors_(0) { }
	
	void DiagnosticRenderer::setColor(std::ostream& stream, const char* color) {
		if (!useColors_) return;
		stream << color;
	}
	
	void DiagnosticRenderer::resetColor(std::ostream& stream) {
		if (!useColors_) return;
		stream << RESET_COLOR;
	}
	
	void DiagnosticRenderer::emitDiagnostic(std::ostream& stream, const Diag& diagnostic,
	                                        const Debug::SourceLocation& location) {
		emitDiagnosticMessage(stream, diagnostic, location);
		emitCodeContext(stream, location);
	}
	
	void DiagnosticRenderer::emitDiagnosticMessage(std::ostream& stream, const Diag& diagnostic,
	                                               const Debug::SourceLocation& location) {
		setColor(stream, MESSAGE_COLOR);
		stream << location.fileName().c_str() << ":";
		stream << location.range().start().lineNumber() << ":";
		stream << location.range().start().column() << ": ";
		resetColor(stream);
		emitDiagnosticLevel(stream, diagnostic.level());
		setColor(stream, MESSAGE_COLOR);
		stream << diagnostic.toString();
		resetColor(stream);
		stream << "\n";
	}
	
	void DiagnosticRenderer::emitDiagnosticLevel(std::ostream& stream, const DiagLevel level) {
		switch (level) {
			case DiagLevel::Error:
				setColor(stream, ERROR_COLOR);
				stream << "error";
				numErrors_++;
				break;
			case DiagLevel::Warning:
				setColor(stream, WARNING_COLOR);
				stream << "warning";
				numWarnings_++;
				break;
			case DiagLevel::Notice:
				setColor(stream, NOTE_COLOR);
				stream << "note";
				break;
		}
		stream << ": ";
		resetColor(stream);
	}
	
	void DiagnosticRenderer::emitCodeContext(std::ostream& stream, const Debug::SourceLocation& location) {
		// TODO: this needs a lot of improvement, and should probably
		//       be moved out of here entirely.
		const auto handle = fopen(location.fileName().c_str(), "rb");
		if (handle == NULL) {
			return;
		}
		
		const auto begin = findLineStart(handle, location.range().start().byteOffset());
		const auto end = findLineEnd(handle, location.range().start().byteOffset());
		
		const auto length = end - begin;
		
		std::vector<char> data;
		data.resize(length + 1);
		fseek(handle, begin, SEEK_SET);
		const size_t readSize = fread(data.data(), 1, length, handle);
		if (readSize != length) {
			throw std::runtime_error(makeString("Failed to read string in file '%s'.",
			                                    location.fileName().c_str()));
		}
		
		for (size_t i = 0; i < length; i++) {
			if (data[i] == '\t') {
				data[i] = ' ';
			}
		}
		
		data.at(length) = '\0';
		fclose(handle);
		
		stream << " " << (const char*) data.data() << "\n";
		
		const auto offset = (location.range().start().byteOffset() - begin) + 1;
		for (size_t i = 0; i < offset; i++) {
			stream << " ";
		}
		
		setColor(stream, CARET_COLOR);
		stream << "^";
		
		for (size_t i = (location.range().start().byteOffset() + 1); i < end; i++) {
			stream << "~";
		}
		
		resetColor(stream);
		stream << "\n";
	}
	
	size_t DiagnosticRenderer::findLineStart(FILE* file, size_t begin) {
		while (begin > 0) {
			fseek(file, begin - 1, SEEK_SET);
			const auto c = fgetc(file);
			if (c == '\n' || c == '\r') {
				return begin;
			}
			begin--;
		}
		return begin;
	}
	
	size_t DiagnosticRenderer::findLineEnd(FILE* file, size_t end) {
		while (true) {
			fseek(file, end, SEEK_SET);
			const auto c = fgetc(file);
			if (c == EOF || c == '\n' || c == '\r') {
				return end;
			}
			end++;
		}
	}
	
	void DiagnosticRenderer::emitDiagnosticSummary(std::ostream& stream) {
		if (numWarnings_ > 0) {
			stream << numWarnings_ << " warning" << (numWarnings_ > 1 ? "s" : "");
		}
		if (numWarnings_ > 0 && numErrors_ > 0) {
			stream << " and ";
		}
		if (numErrors_ > 0) {
			stream << numErrors_ << " error" << (numErrors_ > 1 ? "s" : "");
		}
		if (numWarnings_ > 0 || numErrors_ > 0) {
			stream << " generated.\n";
		}
	}
	
}
