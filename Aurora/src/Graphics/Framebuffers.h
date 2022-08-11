#pragma once

/*
 * Framebuffers are a very useful tool, they can be used to renderer the scene into a texture which is our case, we can also use them for
 * alot of post-processing effects and other stuff
 * As for the depth attachment for framebuffers, via this implementation, it is defaulted to use render buffers for the Depth/Stencil 
 * attachment and that is because these attachments are rarely to ever be directly read from therefore it is more optimized to attach them
 * as render buffers.
 * TODO: Add the choice to specify whether to use  render buffers are texture2Ds for the depth and stencil attachments
 */

#include "Core/Base.h"
#include "Texture.h"

#include <initializer_list>
#include <glm/glm.hpp>

namespace Aurora {

	// TODO: Add more formats when the need arises
	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth / Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format)
			: TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		//TextureWrap Wrap; // TODO: add it when mousepicking is done
		//TextureFilter Filter;
	};

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(const std::initializer_list<FrameBufferTextureSpecification>& attachments)
			: Attachments(attachments) {}

		std::vector<FrameBufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		// glm::vec4 ClearColor; // TODO: Work out how this is going to work, each FB should have its own clear value
		FrameBufferAttachmentSpecification Attachments;
		uint32_t Samples = 1; // This is for multisampling and anit-aliasing

		bool SwapChainTarget = false; // This is the equivalent of glBindFramebuffer(0);, however that is for vulkan most probably
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		~Framebuffer();

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);
		int ReadPixel(uint32_t attachmentIndex, int x, int y);

		void Bind() const;
		void UnBind() const;

		void ClearTextureAttachment(uint32_t attachmentIndex, int data) const;

		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		uint32_t GetColorAttachmentID(uint32_t index = 0) const { AR_CORE_ASSERT(index < m_ColorAttachments.size(), "Framebuffer", "Index cant be greater than the size");  return m_ColorAttachments[index]; }

	private:
		uint32_t m_BufferID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentsSpecification;
		FrameBufferTextureSpecification m_DepthAttachmentSpecification = FrameBufferTextureFormat::None; // It does not make sense to have 2 depth buffers, i dont know if thats even possible

		std::vector<uint32_t> m_ColorAttachments; // This is our color attachments IDs
		uint32_t m_DepthAttachment = 0;

	};

}