#pragma once

/*
 * This is a very very very basic implementation of a reference counting system and is very temporary, and will be changed when I get
 * access to hazel-dev and look at an actual real reference counting system and implementation, however for now, i will use my own 
 * idea and implementation of a ref counting system.. which is just to get read of all the shared pointers situation
 */

#include "Logging/Log.h"

template<typename... > struct typelist;

namespace Aurora {

	template<typename T>
	class RefCountedObject
	{
	public:
		RefCountedObject(T* ptr = nullptr)
			: m_Ptr(ptr)
		{
			if (m_Ptr)
				m_RefCount = new uint32_t(1);
			else
				m_RefCount = nullptr;
		}

		template<typename... Args, typename = std::enable_if_t<!std::is_same<typelist<RefCountedObject>, typelist<std::decay_t<Args>...>>::value>>
		RefCountedObject(Args&&... args)
		{
			m_Ptr = new T(std::forward<Args>(args)...);
			m_RefCount = new uint32_t(1);
		}

		RefCountedObject(const RefCountedObject& other)
			: m_Ptr(other.m_Ptr), m_RefCount(other.m_RefCount)
		{
			(*m_RefCount)++;
		}

		RefCountedObject(RefCountedObject&& other) noexcept
			: m_Ptr(other.m_Ptr), m_RefCount(other.m_RefCount)
		{
			other.m_Ptr = nullptr;
			other.m_RefCount = nullptr;
		}

		~RefCountedObject()
		{
			if (m_Ptr && --(*m_RefCount) <= 0)
			{
				delete m_Ptr;
				delete m_RefCount;
			}
		}

		RefCountedObject& operator=(const RefCountedObject& other)
		{
			m_Ptr = other.m_Ptr;
			m_RefCount = other.m_RefCount;
			(*m_RefCount)++;

			return *this;
		}

		RefCountedObject& operator=(RefCountedObject&& other) noexcept
		{
			m_Ptr = other.m_Ptr;
			m_RefCount = other.m_RefCount;

			other.m_Ptr = nullptr;
			other.m_RefCount = nullptr;

			return *this;
		}

		uint32_t GetRefCount() const { return m_RefCount ? *m_RefCount : 0; }
		bool IsUnique() const { return *m_RefCount == 0; }

		operator T* () const { return m_Ptr; }
		T* operator&() const { return m_Ptr; }

		T& operator*() const { return *m_Ptr; }
		T* operator->() const { return m_Ptr; }

	private:
		T* m_Ptr;
		uint32_t* m_RefCount;

	};

	template<typename T, typename... Args>
	[[nodiscard]]
	RefCountedObject<T> CreateReferencedObject(Args&&... args)
	{
		RefCountedObject<T> obj(std::forward<Args>(args)...);

		return obj;
	}

}