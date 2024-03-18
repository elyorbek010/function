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
			: size(0)
		{ }

		function(std::nullptr_t) noexcept
			: size(0)
		{ }

		function(const function& other)
			: size(other.size)
		{
			if (other.size)
			{
				other.pimpl()->clone(pimpl());
			}
		}

		function(function&& other)
			: size(other.size)
		{
			if (other.size)
			{
				other.pimpl()->move(pimpl());
				other.size = 0;
			}
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function(FunctionT&& function)
			: size(sizeof (FunctionModel<FunctionT>))
		{
			::new (pimpl()) FunctionModel<FunctionT>(std::forward<FunctionT>(function));
		}

		// Assignment operator overloading
		function& operator=(std::nullptr_t) noexcept
		{
			size = 0;
			std::fill_n(buffer, sizeof buffer, 0);
			return *this;
		}

		function& operator=(const function& rhs)
		{
			if (size = rhs.size)
			{
				function copy(rhs);
				swap(buffer, copy.buffer);
			}

			return *this;
		}

		function& operator=(function&& rhs)
		{
			if (size = rhs.size)
			{
				function tmp(std::move(rhs));
				swap(buffer, tmp.buffer);
				rhs.size = 0;
			}

			return *this;
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function& operator=(FunctionT&& rhs)
		{
			function tmp(rhs);
			swap(buffer, tmp.buffer);
			size = tmp.size;

			return *this;
		}

		// Comparison operators overloading
		explicit operator bool() const noexcept
		{
			return size;
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
			if (!size)
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

		static constexpr size_t capacity = 128;
		size_t size;

		alignas(16) std::byte buffer[capacity];
		
	};

}

#endif // FUNCTION_H