#pragma once

#include "Core/Base.h"
#include "Framebuffers.h"

namespace Aurora {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		std::string DebugName;
	};

	class RenderPass : public RefCountedObject
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass();

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);

		RenderPassSpecification& GetSpecification() { return m_Specification; }
		const RenderPassSpecification& GetSpecification() const { return m_Specification; }

	private:
		RenderPassSpecification m_Specification;

	};

}