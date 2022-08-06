#pragma once

/*
 * This is a very very very basic implementation of a reference counting system and is very temporary, and will be changed when I get
 * access to hazel-dev and look at an actual real reference counting system and implementation, however for now, i will use my own 
 * idea and implementation of a ref counting system.. which is just to get rid of all the shared pointers situation.
 * This does not support pointers to arrays for now and probably never will untill the use case arrises, then i would just implement it.
 * Create Weak_Pointers
 */

template<typename... > struct typelist;

namespace Aurora {

	template<typename T>
	class RefCountedObject
	{
	public:
		constexpr RefCountedObject() noexcept = default;
		constexpr RefCountedObject(std::nullptr_t) noexcept {}

		RefCountedObject(T* ptr)
			: m_Ptr(ptr)
		{
			if (m_Ptr)
				m_RefCount = new uint32_t(1);
			else
				m_RefCount = nullptr;
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

		T* raw() const { return m_Ptr; }
		uint32_t GetRefCount() const { return m_RefCount ? *m_RefCount : 0; }
		bool IsUnique() const { return *m_RefCount == 0; }

		bool operator==(const RefCountedObject& other) const { return m_Ptr == other.m_Ptr; }
		bool operator!=(const RefCountedObject& other) const { return !(*this == other); }

		operator bool() const { return m_Ptr != nullptr; }
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
		RefCountedObject<T> obj = RefCountedObject<T>(new T(std::forward<Args>(args)...));

		return obj;
	}


	template<typename T>
	class ScopedPointer
	{
	public:
		constexpr ScopedPointer() noexcept = default;
		constexpr ScopedPointer(std::nullptr_t) noexcept {}

		ScopedPointer(T* ptr)
			: m_Ptr(ptr) {}

		ScopedPointer(const ScopedPointer&) = delete;

		ScopedPointer(ScopedPointer&& other)
		{
			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;
		}

		~ScopedPointer()
		{
			if (m_Ptr)
				delete m_Ptr;
		}

		ScopedPointer& operator=(const ScopedPointer&) = delete;

		ScopedPointer& operator=(ScopedPointer&& other)
		{
			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;

			return *this;
		}


		bool operator==(const ScopedPointer& other) const { return m_Ptr == other.m_Ptr; }
		bool operator!=(const ScopedPointer& other) const { return !(*this == other); }

		operator T* () const { return m_Ptr; }
		T* operator&() const { return m_Ptr; }

		T& operator*() const { return *m_Ptr; }
		T* operator->() const { return m_Ptr; }

	private:
		T* m_Ptr;

	};

	template<typename T, typename... Args>
	[[nodiscard]]
	ScopedPointer<T> CreateScopedPointer(Args&&... args)
	{
		ScopedPointer<T> obj(new T(std::forward<Args>(args)...));

		return obj;
	}

}