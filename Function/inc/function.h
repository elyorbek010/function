#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdexcept>

namespace my {

template< class ReturnType, class... Args >
class function;

template< class ReturnType, class... Args >
class function<ReturnType(Args...)>
{
private: // Helper classes

	class FunctionConcept
	{
	public:
		virtual ~FunctionConcept() = default;

		virtual FunctionConcept* clone() const = 0;
		virtual void clone(FunctionConcept* memory) const = 0;
		virtual void move(FunctionConcept* memory) = 0;
		virtual ReturnType operator()(Args... args) const = 0;

	}; // class FunctionConcept

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
			new (memory) FunctionModel(*this);
		}

		void move(FunctionConcept* memory) override
		{
			new (memory) FunctionModel(std::move(*this));
		}

		ReturnType operator()(Args... args) const override
		{
			return function_(args...);
		}

	private:
		const FunctionT function_;

	}; // class FunctionModel

public: // Special member functions

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
		if (other.is_small_buffer())
			other.get_pimpl()->clone(get_pimpl());
		else
			set_pimpl(other.get_pimpl()->clone());
	}

	function(function&& other)
		: size(other.size), storage()
	{
		if (other.is_small_buffer())
			other.get_pimpl()->move(get_pimpl());
		else
			set_pimpl(other.get_pimpl());

		other.size = 0;
	}

	template<class FunctionT, class = std::enable_if_t<!std::is_same<function, std::remove_reference_t<FunctionT>>::value, function>>
	function(FunctionT&& function)
		: size(sizeof(FunctionModel<std::decay_t<FunctionT>>)), storage()
	{
		if (is_small_buffer())
			new (get_pimpl()) FunctionModel<std::decay_t<FunctionT>>(std::forward<FunctionT>(function));
		else
			set_pimpl(new FunctionModel<std::decay_t<FunctionT>>(std::forward<FunctionT>(function)));
	}

	// Destructor
	~function()
	{
		if (is_empty())
			return;

		if (!is_small_buffer())
			delete storage.ptr;
		else
			get_pimpl()->~FunctionConcept();
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

	// Function call operator
	ReturnType operator()(Args... args) const
	{
		if (is_empty())
			throw std::runtime_error("empty object");

		return (*get_pimpl())(args...);
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

private: // Helper member functions

	void swap(function& other)
	{
		std::swap(storage, other.storage);
		std::swap(size, other.size);
	}

	template<typename T>
	T* get_pimpl_impl() const noexcept {
		if (is_small_buffer())
			return reinterpret_cast<T*>(const_cast<std::byte*>(storage.buffer));
		else
			return storage.ptr;
	}

	FunctionConcept* get_pimpl() noexcept {
		return get_pimpl_impl<FunctionConcept>();
	}

	const FunctionConcept* get_pimpl() const noexcept {
		return get_pimpl_impl<const FunctionConcept>();
	}

	void set_pimpl(FunctionConcept* ptr) noexcept
	{
		storage.ptr = ptr;
	}

	bool is_small_buffer() const
	{
		return size <= capacity;
	}

	bool is_empty() const
	{
		return size == 0;
	}

private: // Member variables

	static constexpr size_t capacity = 2 * sizeof(void *);
	size_t size{0};

	alignas(max_align_t) union Storage
	{
		std::byte buffer[capacity];
		FunctionConcept* ptr;
		
		Storage() { }
		~Storage() { }
	} storage;

}; // class function

} // namespace my

#endif // FUNCTION_H