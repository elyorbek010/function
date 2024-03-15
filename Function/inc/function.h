#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include <type_traits>
#include <cstring>

namespace my {

	template< class ReturnType, class... Args >
	class function;

	template< class ReturnType, class... Args >
	class function<ReturnType(Args...)>
	{
	public:
		// Constructors
		function() noexcept
			: is_empty(true)
		{ }

		function(std::nullptr_t) noexcept
			: is_empty(true)
		{ }

		function(const function& other)
		{
			if (!(is_empty = other.is_empty))
				other.pimpl()->clone(pimpl());
		}

		function(function&& other)
		{
			if (!(is_empty = other.is_empty))
				other.pimpl()->move(pimpl());
			other.is_empty = true;
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function(FunctionT&& function)
			: is_empty(false)
		{
			::new (pimpl()) FunctionModel<FunctionT>(std::forward<FunctionT>(function));
		}

		// Assignment operator overloading
		function& operator=(std::nullptr_t) noexcept
		{
			is_empty = true;
			std::fill_n(buffer, sizeof buffer, 0);
			return *this;
		}

		function& operator=(const function& rhs)
		{
			if (!(is_empty = rhs.is_empty))
			{
				function copy(rhs);
				swap(buffer, copy.buffer);
			}

			return *this;
		}

		function& operator=(function&& rhs)
		{
			if (!(is_empty = rhs.is_empty))
			{
				function tmp(std::move(rhs));
				swap(buffer, tmp.buffer);
			}
			rhs.is_empty = true;

			return *this;
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function& operator=(FunctionT&& rhs)
		{
			function tmp(rhs);
			swap(buffer, tmp.buffer);
			is_empty = false;

			return *this;
		}

		// Comparison operators overloading
		explicit operator bool() const noexcept
		{
			return !is_empty;
		}

		bool operator==(std::nullptr_t) const noexcept
		{
			return !(this->operator bool());
		}

		bool operator!=(std::nullptr_t) const noexcept
		{
			return this->operator bool();
		}

		ReturnType operator()(Args... args) const
		{
			if (is_empty)
				throw std::runtime_error("empty object");

			return (*pimpl())(args...);
		}

		~function() = default;

	private:
		class FunctionConcept
		{
		public:
			virtual ~FunctionConcept() = default;

			virtual void clone(FunctionConcept* memory) const = 0;
			virtual void move(FunctionConcept* memory) const = 0;
			virtual ReturnType operator()(Args... args) const = 0;
		};

		template<class FunctionT>
		class FunctionModel : public FunctionConcept
		{
		public:
			FunctionModel(const FunctionT& function) :
				function_(function)
			{ }

			void clone(FunctionConcept* memory) const override
			{
				::new (memory) FunctionModel(*this);
			}

			void move(FunctionConcept* memory) const override
			{
				::new (memory) FunctionModel(std::move(*this));
			}

			ReturnType operator()(Args... args) const override
			{
				return function_(args...);
			}

		private:
			FunctionT function_;
		};

		FunctionConcept* pimpl() noexcept
		{
			return reinterpret_cast<FunctionConcept*>(buffer);
		}

		const FunctionConcept* pimpl() const noexcept
		{
			return reinterpret_cast<const FunctionConcept*>(buffer);
		}

		alignas(16) std::byte buffer[128];
		bool is_empty;
	};

}

#endif // FUNCTION_H