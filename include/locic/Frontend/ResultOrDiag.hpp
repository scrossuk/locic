#ifndef LOCIC_FRONTEND_RESULTORDIAG_HPP
#define LOCIC_FRONTEND_RESULTORDIAG_HPP

#include <memory>
#include <type_traits>

#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Frontend/OptionalDiag.hpp>

namespace locic {
	
	template <typename T>
	class ResultOrDiag {
	public:
		ResultOrDiag(T argValue)
		: isSuccess_(true) {
			new(&value_) T(std::move(argValue));
		}
		
		ResultOrDiag(Diag argDiag)
		: isSuccess_(false),
		diag_(std::unique_ptr<DiagAPI>(new Diag(std::move(argDiag)))) { }
		
		ResultOrDiag(std::unique_ptr<DiagAPI> argDiag)
		: isSuccess_(false), diag_(std::move(argDiag)) { }
		
		ResultOrDiag(OptionalDiag argDiag)
		: isSuccess_(false), diag_(argDiag.extractDiag()) { }
		
		~ResultOrDiag() {
			if (success()) { value().~T(); }
		}
		
		ResultOrDiag(ResultOrDiag<T>&& other)
		: isSuccess_(other.success()) {
			if (other.success()) {
				new(&value_) T(std::move(other.value()));
			} else {
				diag_ = std::move(other.diag_);
			}
		}
		
		ResultOrDiag<T>& operator=(ResultOrDiag<T>&& other) {
			if (this != &other) {
				this->~ResultOrDiag();
				new(this) ResultOrDiag(std::move(other));
			}
			return *this;
		}
		
		bool success() const { return isSuccess_; }
		bool failed() const { return !success(); }
		
		T& value() {
			assert(success());
			return *(static_cast<T*>(static_cast<void*>(&value_)));
		}
		
		const T& value() const {
			assert(success());
			return *(static_cast<const T*>(static_cast<const void*>(&value_)));
		}
		
		const DiagAPI& diag() const {
			assert(failed());
			return *diag_;
		}
		
		template <typename Other>
		operator ResultOrDiag<Other>() {
			assert(failed());
			return ResultOrDiag<Other>(std::move(diag_));
		}
		
		operator OptionalDiag() {
			if (success()) {
				return SUCCESS;
			} else {
				return OptionalDiag(std::move(diag_),
				                    Debug::SourceLocation::Null(),
				                    OptionalDiag());
			}
		}
		
	private:
		bool isSuccess_;
		typename std::aligned_storage<sizeof(T), alignof(T)>::type value_;
		std::unique_ptr<DiagAPI> diag_;
		
	};
	
}

#endif
