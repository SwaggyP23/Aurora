#pragma once

/*
 * This is a very very very basic implementation of a reference counting system and is very temporary, and will be changed when I get
 * access to hazel-dev and look at an actual real reference counting system and implementation, however for now, i will use my own 
 * idea and implementation of a ref counting system.. which is just to get rid of all the shared pointers situation.
 * This does not support pointers to arrays for now and probably never will untill the use case arrises, then i would just implement it.
 * Create Weak_Pointers
 */

namespace Aurora {

	class RefCountedObject
	{
	public:
		void IncrementRefCount() const;
		void DecrementRefCount() const;

		uint32_t GetRefCount() const { return m_RefCount.load(); }

	private:
		mutable std::atomic<uint32_t> m_RefCount = 0;

	};

	// Here we keep a set of all the live references in the engine, whenever we want to know if a certain reference is live, you can just call IsLive(ptr)
	namespace RefUtils {

		void AddToLiveReference(void* ptr);
		void RemoveFromLiveReferences(void* ptr);
		bool IsLive(void* ptr);

	}

	template<typename T>
	class Ref
	{
	public:
		constexpr Ref() noexcept = default;
		constexpr Ref(std::nullptr_t) noexcept 
		: m_Ptr(nullptr) {}

		Ref(T* ptr)
			: m_Ptr(ptr)
		{
			static_assert(std::is_base_of<RefCountedObject, T>::value, "Class is  not a RefCountedObject!");

			IncrementRef();
		}

		Ref(const Ref<T>& other)
		{
			m_Ptr = other.m_Ptr;
			IncrementRef();
		}

		Ref(Ref<T>&& other)
		{
			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;
		}

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		Ref(const Ref<T2>& other)
		{
			m_Ptr = (T*)other.m_Ptr;
			IncrementRef();
		}

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		Ref(Ref<T2>&& other) noexcept
		{
			m_Ptr = (T*)other.m_Ptr;
			other.m_Ptr = nullptr;
		}

		~Ref()
		{
			DecrementRef();
		}

		Ref& operator=(std::nullptr_t)
		{
			DecrementRef();
			m_Ptr = nullptr;
			return *this;
		}

		Ref& operator=(const Ref<T>& other)
		{
			other.IncrementRef();
			DecrementRef();

			m_Ptr = other.m_Ptr;
			return *this;
		}

		Ref& operator=(Ref<T>&& other) noexcept
		{
			DecrementRef();

			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;

			return *this;
		}

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncrementRef();
			DecrementRef();

			m_Ptr = (T*)other.m_Ptr;
			return *this;
		}

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		Ref& operator=(Ref<T2>&& other) noexcept
		{
			DecrementRef();

			m_Ptr = (T*)other.m_Ptr;
			other.m_Ptr = nullptr;

			return *this;
		}

		void Reset(T* ptr = nullptr)
		{
			DecrementRef();
			m_Ptr = ptr;
		}

		T* raw() { return m_Ptr; }
		const T* raw() const { return m_Ptr; }

		operator bool() const { return m_Ptr != nullptr; }

		bool operator==(const Ref<T>& other) const { return m_Ptr == other.m_Ptr; }
		bool operator!=(const Ref<T>& other) const { return !(*this == other); }

		T& operator*() { return *m_Ptr; }
		const T& operator*() const { return *m_Ptr; }

		T* operator->() { return m_Ptr; }
		const T* operator->() const { return m_Ptr; }

	private:
		void IncrementRef() const
		{
			if (m_Ptr)
			{
				m_Ptr->IncrementRefCount();
				RefUtils::AddToLiveReference((void*)m_Ptr);
			}
		}

		void DecrementRef() const
		{
			if (m_Ptr)
			{
				m_Ptr->DecrementRefCount();
				if (m_Ptr->GetRefCount() == 0)
				{
					delete m_Ptr;
					RefUtils::RemoveFromLiveReferences((void*)m_Ptr);
					m_Ptr = nullptr;
				}
			}
		}

	private:
		template<typename T2>
		friend class Ref;

		mutable T* m_Ptr = nullptr;

	};

	template<typename T, typename... Args>
	[[nodiscard]]
	Ref<T> CreateReferencedObject(Args&&... args)
	{
		return Ref<T>(new T(std::forward<Args>(args)...));
	}


	template<typename T>
	class ScopedPointer
	{
	public:
		constexpr ScopedPointer() noexcept = default;
		constexpr ScopedPointer(std::nullptr_t) noexcept 
		: m_Ptr(nullptr) {}

		ScopedPointer(T* ptr)
		{
			if (ptr)
				m_Ptr = ptr;
		}

		ScopedPointer(const ScopedPointer&) = delete;

		ScopedPointer(ScopedPointer&& other)
		{
			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;
		}

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		ScopedPointer(ScopedPointer<T2>&& other)
		{
			m_Ptr = (T*)other.m_Ptr;
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

		template<typename T2, std::enable_if_t<std::_SP_pointer_compatible<T2, T>::value, int> = 0>
		ScopedPointer& operator=(ScopedPointer<T2>&& other)
		{
			m_Ptr = (T*)other.m_Ptr;
			other.m_Ptr = nullptr;

			return *this;
		}

		operator bool() const { return m_Ptr != nullptr; }

		bool operator==(const ScopedPointer& other) const { return m_Ptr == other.m_Ptr; }
		bool operator!=(const ScopedPointer& other) const { return !(*this == other); }

		T& operator*() { return *m_Ptr; }
		const T& operator*() const { return *m_Ptr; }

		T* operator->() { return m_Ptr; }
		const T* operator->() const { return m_Ptr; }

	private:
		T* m_Ptr = nullptr;

	};

	template<typename T, typename... Args>
	[[nodiscard]]
	ScopedPointer<T> CreateScopedPointer(Args&&... args)
	{
		return ScopedPointer<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T>
	class WeakRef
	{
	public:
		constexpr WeakRef() = default;

		WeakRef(Ref<T> ref)
		{
			m_Ptr = ref.raw();
		}

		WeakRef(T* ptr)
		{
			m_Ptr = ptr;
		}

		bool IsValid()
		{
			return m_Ptr ? RefUtils::IsLive(m_Ptr) : false;
		}

		operator bool() const { return IsValid(); }

	private:
		T* m_Ptr = nullptr;

	};

}