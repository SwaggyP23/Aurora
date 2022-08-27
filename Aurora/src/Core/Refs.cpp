#include "Aurorapch.h"
#include "Refs.h"

namespace Aurora {

	static std::unordered_set<void*> s_LiveReferences;
	static std::mutex s_LiveReferenceMutex;

	void RefCountedObject::IncrementRefCount() const
	{
		++m_RefCount;
	}

	void RefCountedObject::DecrementRefCount() const
	{
		--m_RefCount;
	}

	namespace RefUtils {

		void AddToLiveReference(void* ptr)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			AR_CORE_ASSERT(ptr, "ptr cant be null!");
			s_LiveReferences.insert(ptr);
		}

		void RemoveFromLiveReferences(void* ptr)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			AR_CORE_ASSERT(ptr, "ptr cant be null!");
			s_LiveReferences.erase(ptr);
		}

		bool IsLive(void* ptr)
		{
			AR_CORE_ASSERT(ptr, "ptr cant be null");
			return s_LiveReferences.find(ptr) != s_LiveReferences.end();
		}

	}

}