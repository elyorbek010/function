#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include <type_traits>

namespace my {

template< class ReturnType, class... Args >
class function;

template< class ReturnType, class... Args >
class function<ReturnType(Args...)>
{
public:
	~function() = default;

	// Constructors
	function() noexcept
		: wrappedFunctionPtr(nullptr)
	{ }

	function(std::nullptr_t) noexcept
		: wrappedFunctionPtr(nullptr)
	{ }

	function(const function& other)
		: wrappedFunctionPtr(other.wrappedFunctionPtr->clone())
	{ }

	function(function&& other) = default;

	template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
	function(FunctionT&& function)
		: wrappedFunctionPtr(std::make_unique<FunctionModel<FunctionT>>(std::forward<FunctionT>(function)))
	{ }

	// Assignment operator overloading
	function& operator=(std::nullptr_t) noexcept
	{
		wrappedFunctionPtr = nullptr;
		return *this;
	}

	function& operator=(const function& rhs)
	{
		rhs.wrappedFunctionPtr->clone().swap(this->wrappedFunctionPtr);
		return *this;
	}

	function& operator=(function&& rhs) = default;
	
	template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
	function& operator=(FunctionT&& function)
	{
		wrappedFunctionPtr = std::make_unique<FunctionModel<FunctionT>>(std::forward<FunctionT>(function));
		return *this;
	}
	
	// Comparison operators overloading
	explicit operator bool() const noexcept
	{
		return wrappedFunctionPtr != nullptr;
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
		if (wrappedFunctionPtr == nullptr)
			throw std::runtime_error("empty object");

		return (*wrappedFunctionPtr)(args...);
	}

private:

	class FunctionConcept
	{
	public:
		virtual ~FunctionConcept() = default;

		virtual std::unique_ptr<FunctionConcept> clone() const = 0;
		virtual ReturnType operator()(Args... args) const = 0;
	};

	template<class FunctionT>
	class FunctionModel : public FunctionConcept
	{
	public:
		FunctionModel(const FunctionT& function) :
			function(function) { }

		std::unique_ptr<FunctionConcept> clone() const override
		{
			return std::make_unique<FunctionModel>(*this);
		}

		ReturnType operator()(Args... args) const override
		{
			return function(args...);
		}

	private:
		FunctionT function;
	};

	std::unique_ptr<FunctionConcept> wrappedFunctionPtr;
};

}

#endif // FUNCTION_H