#include "Aurorapch.h"
#include "RenderPass.h"

namespace Aurora {

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		return CreateRef<RenderPass>(spec);
	}

	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec)
	{
	}

	RenderPass::~RenderPass()
	{
	}

}