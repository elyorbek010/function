#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <stdexcept>

template< class ReturnType, class... Args >
class FunctionWrapper;

template< class ReturnType, class... Args >
class FunctionWrapper<ReturnType(Args...)>
{
public:
	~FunctionWrapper() = default;

	// Constructors
	FunctionWrapper() noexcept
		: wrappedFunctionPtr(nullptr)
	{ }

	FunctionWrapper(std::nullptr_t) noexcept
		: wrappedFunctionPtr(nullptr)
	{ }

	FunctionWrapper(const FunctionWrapper& other)
		: wrappedFunctionPtr(other.wrappedFunctionPtr->clone())
	{ }

	FunctionWrapper(FunctionWrapper& other)
		: wrappedFunctionPtr(other.wrappedFunctionPtr->clone())
	{ }

	FunctionWrapper(FunctionWrapper&& other) = default;

	template<class FunctionType>
	FunctionWrapper(FunctionType&& function)
		: wrappedFunctionPtr(std::make_unique<ConcreteFunction<FunctionType>>(std::forward<FunctionType>(function)))
	{ }

	// Assignment operator overloading
	FunctionWrapper& operator=(std::nullptr_t) noexcept
	{
		wrappedFunctionPtr = nullptr;
		return *this;
	}

	FunctionWrapper& operator=(const FunctionWrapper& rhs)
	{
		rhs.wrappedFunctionPtr->clone().swap(this->wrappedFunctionPtr);
		return *this;
	}

	FunctionWrapper& operator=(FunctionWrapper& rhs)
	{
		rhs.wrappedFunctionPtr->clone().swap(this->wrappedFunctionPtr);
		return *this;
	}

	FunctionWrapper& operator=(FunctionWrapper&& rhs) = default;
	
	template<class FunctionType>
	FunctionWrapper& operator=(FunctionType&& function)
	{
		wrappedFunctionPtr = std::make_unique<ConcreteFunction<FunctionType>>(std::forward<FunctionType>(function));
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

	class FunctionBase
	{
	public:
		virtual ~FunctionBase() = default;

		virtual std::unique_ptr<FunctionBase> clone() const = 0;
		virtual ReturnType operator()(Args... args) const = 0;
	};

	template<class FunctionType>
	class ConcreteFunction : public FunctionBase
	{
	public:
		ConcreteFunction(const FunctionType& function) :
			functionObject(function) { }

		std::unique_ptr<FunctionBase> clone() const override
		{
			return std::make_unique<ConcreteFunction>(*this);
		}

		ReturnType operator()(Args... args) const override
		{
			return functionObject(args...);
		}

	private:
		FunctionType functionObject;
	};

	std::unique_ptr<FunctionBase> wrappedFunctionPtr;
};

#endif // FUNCTION_H