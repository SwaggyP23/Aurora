#pragma once

/*
 * Framebuffers are a very useful tool, they can be used to renderer the scene into a texture which is our case, we can also use them for
 * alot of post-processing effects and other stuff
 */

#include "Core/Base.h"

namespace Aurora {

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		uint32_t Samples = 1;

		bool SwapChainTarget = false; // This is the equivalent of glBindFramebuffer(0);
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		~Framebuffer();

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);

		void bind() const;
		void unBind() const;

		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		uint32_t GetColorAttachmentID() const { return m_ColorAttachment; }

	private:
		uint32_t m_BufferID = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;
		FramebufferSpecification m_Specification;

	};

}