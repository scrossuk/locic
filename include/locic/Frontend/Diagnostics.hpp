#ifndef LOCIC_FRONTEND_DIAGNOSTICS_HPP
#define LOCIC_FRONTEND_DIAGNOSTICS_HPP

#include <string>

#include <locic/Support/ErrorHandling.hpp>

namespace locic {
	
	namespace Lex {
		enum class DiagID;
	}
	
	enum class DiagLevel {
		Error,
		Warning,
		Notice
	};
	
	class DiagAPI {
	public:
		virtual ~DiagAPI() { }
		
		virtual DiagLevel level() const = 0;
		
		virtual Lex::DiagID lexId() const {
			locic_unreachable("Not lex diagnostic.");
		}
		
		virtual std::string toString() const = 0;
		
	protected:
		DiagAPI() = default;
		DiagAPI(const DiagAPI&) = default;
		DiagAPI& operator=(const DiagAPI&) = default;
		DiagAPI(DiagAPI&&) = default;
		DiagAPI& operator=(DiagAPI&&) = default;
		
	};
	
	class Diag: public DiagAPI {
	public:
		Diag(DiagLevel level, std::string text)
		: level_(level), text_(std::move(text)) { }
		
		DiagLevel level() const {
			return level_;
		}
		
		std::string toString() const {
			return text_;
		}
		
	private:
		DiagLevel level_;
		std::string text_;
		
	};
	
	class ErrorDiag: public DiagAPI {
	public:
		DiagLevel level() const {
			return DiagLevel::Error;
		}
	};
	
	class WarningDiag: public DiagAPI {
	public:
		DiagLevel level() const {
			return DiagLevel::Warning;
		}
	};
	
	class NoticeDiag: public DiagAPI {
	public:
		DiagLevel level() const {
			return DiagLevel::Notice;
		}
	};
	
	template <typename... Args>
	Diag Error(const char* const str, const Args&... args) {
		return Diag(DiagLevel::Error, makeString(str, args...));
	}
	
	template <typename... Args>
	Diag Warning(const char* const str, const Args&... args) {
		return Diag(DiagLevel::Warning, makeString(str, args...));
	}
	
	template <typename... Args>
	Diag Notice(const char* const str, const Args&... args) {
		return Diag(DiagLevel::Notice, makeString(str, args...));
	}
	
}

#endif
