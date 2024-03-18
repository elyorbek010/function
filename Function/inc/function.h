#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include <type_traits>
#include <cstring>
#include <algorithm>

namespace my {

	template< class ReturnType, class... Args >
	class function;

	template< class ReturnType, class... Args >
	class function<ReturnType(Args...)>
	{
	public:
		// Constructors
		function() noexcept
			: size(0), storage()
		{ }

		function(std::nullptr_t) noexcept
			: size(0), storage()
		{ }

		function(const function& other)
			: size(other.size), storage()
		{
			if (other.is_sb())
				other.get_pimpl()->clone(get_pimpl());
			else
				set_pimpl(other.get_pimpl()->clone());
		}

		function(function&& other)
			: size(other.size), storage()
		{
			if (other.is_sb())
				other.get_pimpl()->move(get_pimpl());
			else
				set_pimpl(other.get_pimpl());

			other.size = 0;
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function(FunctionT&& function)
			: size(sizeof(FunctionModel<FunctionT>)), storage()
		{
			if (is_sb())
				::new (get_pimpl()) FunctionModel<FunctionT>(std::forward<FunctionT>(function));
			else
				set_pimpl(new FunctionModel<FunctionT>(std::forward<FunctionT>(function)));
		}

		// Assignment operator overloading
		function& operator=(std::nullptr_t) noexcept
		{
			function tmp(nullptr);
			tmp.swap(*this);

			return *this;
		}

		function& operator=(const function& rhs)
		{
			function tmp(rhs);
			tmp.swap(*this);

			return *this;
		}

		function& operator=(function&& rhs)
		{
			function tmp(std::move(rhs));
			tmp.swap(*this);

			return *this;
		}

		template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
		function& operator=(FunctionT&& rhs)
		{
			function tmp(std::move(rhs));
			tmp.swap(*this);

			return *this;
		}

		// Comparison operators overloading
		explicit operator bool() const noexcept
		{
			return !is_empty();
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

			return (*get_pimpl())(args...);
		}
		
		void swap(function& other)
		{
			Storage tmp_storage = other.storage;
			other.storage = storage;
			storage = tmp_storage;
			std::swap(size, other.size);
		}

		~function() = default;

	private:

		class FunctionConcept
		{
		public:
			virtual ~FunctionConcept() = default;

			virtual FunctionConcept* clone() const = 0;
			virtual void clone(FunctionConcept* memory) const = 0;
			virtual void move(FunctionConcept* memory) = 0;
			virtual ReturnType operator()(Args... args) const = 0;
		};

		template<class FunctionT>
		class FunctionModel : public FunctionConcept
		{
		public:
			FunctionModel(const FunctionT& function) :
				function_(function)
			{ }

			FunctionConcept* clone() const override
			{
				return new FunctionModel(*this);
			}

			void clone(FunctionConcept* memory) const override
			{
				::new (memory) FunctionModel(*this);
			}

			void move(FunctionConcept* memory) override
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

		FunctionConcept* get_pimpl() noexcept
		{
			if (is_sb())
				return reinterpret_cast<FunctionConcept*>(storage.buffer);
			else
				return storage.ptr;
		}

		const FunctionConcept* get_pimpl() const noexcept
		{
			if (is_sb())
				return reinterpret_cast<const FunctionConcept*>(storage.buffer);
			else
				return storage.ptr;
		}

		void set_pimpl(FunctionConcept* ptr) noexcept
		{
			storage.ptr = ptr;
		}

		bool is_sb() const // is small buffer
		{
			return size <= capacity;
		}

		bool is_empty() const
		{
			return size == 0;
		}

		static constexpr size_t capacity = 16;
		size_t size;

		union Storage
		{
			alignas(16) std::byte buffer[capacity];
			FunctionConcept* ptr;

			Storage() { }
			~Storage() { }
		} storage;
		
	};

}

#endif // FUNCTION_H