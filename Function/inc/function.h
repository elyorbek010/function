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
		: wrappedFunctionPtr(other.wrappedFunctionPtr)
	{ }

	FunctionWrapper(FunctionWrapper& other)
		: wrappedFunctionPtr(other.wrappedFunctionPtr)
	{ }

	FunctionWrapper(FunctionWrapper&& other) noexcept
		: wrappedFunctionPtr(std::move(other.wrappedFunctionPtr))
	{ }

	template<class FunctionType>
	FunctionWrapper(FunctionType&& function)
		: wrappedFunctionPtr(std::make_shared<ConcreteFunction<FunctionType>>(std::forward<FunctionType>(function)))
	{ }

	// Assignment operator overloading
	FunctionWrapper& operator=(std::nullptr_t) noexcept
	{
		wrappedFunctionPtr = nullptr;
		return *this;
	}

	FunctionWrapper& operator=(const FunctionWrapper& other)
	{
		wrappedFunctionPtr = other.wrappedFunctionPtr;
		return *this;
	}

	FunctionWrapper& operator=(FunctionWrapper& other)
	{
		wrappedFunctionPtr = other.wrappedFunctionPtr;
		return *this;
	}

	FunctionWrapper& operator=(FunctionWrapper&& other)
	{
		wrappedFunctionPtr = std::move(other.wrappedFunctionPtr);
		return *this;
	}
	
	template<class FunctionType>
	FunctionWrapper& operator=(FunctionType&& function)
	{
		wrappedFunctionPtr = std::make_shared<ConcreteFunction<FunctionType>>(std::forward<FunctionType>(function));
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

	// Invoke operator overloading
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
		FunctionBase() = default;
		virtual ~FunctionBase() = default;

		virtual ReturnType operator()(Args... args) const = 0;
	};

	template<class FunctionType>
	class ConcreteFunction : public FunctionBase
	{
	public:
		ConcreteFunction(const FunctionType& function) :
			functionObject{ function } { }

		ReturnType operator()(Args... args) const override
		{
			return functionObject(args...);
		}

	private:
		FunctionType functionObject;
	};

	std::shared_ptr<FunctionBase> wrappedFunctionPtr;
};

#endif // FUNCTION_H