#pragma once

#include "AssetMetaData.h"

#include <unordered_map>

namespace Aurora {

	class AssetRegistry
	{
	public:
		AssetRegistry() = default;
		~AssetRegistry() = default;

		AssetMetaData& operator[](const AssetHandle& handle);
		const AssetMetaData& Get(const AssetHandle& handle) const;

		size_t Count() const { return m_AssetRegistry.size(); }
		bool Contains(const AssetHandle& handle) const;
		size_t Remove(const AssetHandle& handle);
		void Clear();

		auto begin() { return m_AssetRegistry.begin(); }
		auto end() { return m_AssetRegistry.end(); }
		auto cbegin() { return m_AssetRegistry.cbegin(); }
		auto cend() { return m_AssetRegistry.cend(); }

	private:
		std::unordered_map<AssetHandle, AssetMetaData> m_AssetRegistry;

	};

}