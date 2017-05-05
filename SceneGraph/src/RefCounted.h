// Copyright (c) 2007,2010, Eduard Heidt

#pragma once
#include <memory>
#include <functional>

namespace eh
{
	class RefCounted : public std::enable_shared_from_this<RefCounted>
	{
	public:
		bool _shared = false;
	private:
		RefCounted(const RefCounted&) = delete;
	protected:
		RefCounted() {}
		virtual ~RefCounted() {}
	};

	//template<typename T = RefCounted>
	//using Ptr = std::shared_ptr<T>;

	template<typename T = RefCounted>
	class Ptr : public std::shared_ptr<T>
	{
	public:
		Ptr() : std::shared_ptr<T>()
		{}

		Ptr(std::nullptr_t) : std::shared_ptr<T>(nullptr)
		{}

		Ptr(T* p) : std::shared_ptr<T>(p->_shared ? std::dynamic_pointer_cast<T>(p->shared_from_this()) : std::shared_ptr<T>(p))
		{
			p->_shared = true;
		}
		virtual ~Ptr()
		{
		}

		//Ptr(const Ptr<T>& p) : std::shared_ptr<T>(p)
		//{
		//	assert(this->get() == p.get());
		//}

		template<typename U>
		Ptr(const Ptr<U>& p) : std::shared_ptr<T>(std::dynamic_pointer_cast<T>(p))
		{
		}

		operator size_t() const
		{
			return (size_t)this->get();
		}
	};

}
