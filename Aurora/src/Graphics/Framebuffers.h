#pragma once

/*
 * Framebuffers are a very useful tool, they can be used to renderer the scene into a texture which is our case, we can also use them for
 * alot of post-processing effects and other stuff
 * As for the depth attachment for framebuffers, via this implementation, it is defaulted to use render buffers for the Depth/Stencil 
 * attachment and that is because these attachments are rarely to ever be directly read from therefore it is more optimized to attach them
 * as render buffers.
 * TODO: Currently I dont think we support pure Stencil Attachments, however we do support Depth/Stencil attachments
 */

#include "Core/Base.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <initializer_list>

namespace Aurora {

	struct FramebufferTextureSpecification
	{
		constexpr FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(ImageFormat format)
			: TextureFormat(format) {}
		FramebufferTextureSpecification(ImageFormat format, TextureWrap wrapMode)
			: TextureFormat(format), WrapMode(wrapMode) {}
		FramebufferTextureSpecification(ImageFormat format, TextureWrap wrapMode, TextureFilter filteringMode)
			: TextureFormat(format), WrapMode(wrapMode), FilterMode(filteringMode) {}
		FramebufferTextureSpecification(ImageFormat format, TextureFilter filteringMode, TextureWrap wrapMode)
			: TextureFormat(format), FilterMode(filteringMode), WrapMode(wrapMode) {}

		ImageFormat TextureFormat = ImageFormat::None;
		TextureWrap WrapMode = TextureWrap::Clamp;
		TextureFilter FilterMode = TextureFilter::Linear;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		std::string DebugName;
		uint32_t Width = 1280;
		uint32_t Height = 720;
		glm::vec4 ClearColor = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
		uint32_t Samples = 1; // MSAA
		FramebufferAttachmentSpecification AttachmentsSpecification;

		// This sets if the framebuffer is resizable or not
		bool Resizable = true;

		bool ClearOnBind = true;

		// Provides the ability to attach existing images instead of creating new ones
		// The index provided should map exactly with the layout provided to the spec
		// And also the number of samples should also map exactly with the framebuffer it is being referenced from
		std::map<uint32_t, Ref<Texture2D>> ExistingImages;

		// Set this to true to render to the defualt framebuffer
		bool SwapChainTarget = false;
	};

	class Framebuffer : public RefCountedObject
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		virtual ~Framebuffer();

		[[nodiscard]] static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
		[[nodiscard]] static void Blit(uint32_t src, uint32_t dst, uint32_t srcWidth, uint32_t srcHeight, uint32_t srcAttachment, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstAttachment, bool depth = false);

		void Resize(uint32_t width, uint32_t height);

		void Bind() const;
		void UnBind() const;

		void GetColorAttachmentData(void* pixels, uint32_t attachmentIndex = 0);
		void ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, void* data);
		void ClearTextureAttachment(uint32_t attachmentIndex, const void* data);

		[[nodiscard]] uint32_t GetFramebufferID() const { return m_FrameBufferID; }
		[[nodiscard]] const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		[[nodiscard]] Ref<Texture2D> GetColorAttachment(uint32_t index = 0) const { AR_CORE_ASSERT(index < m_ColorAttachments.size(), "Index cant be greater than the size");  return m_ColorAttachments[index]; }
		[[nodiscard]] Ref<Texture2D> GetDepthAttachment() { AR_CORE_ASSERT(m_DepthAttachment); return m_DepthAttachment; }

		[[nodiscard]] bool HasDepthAttachment() const { return m_DepthAttachment ? true : false; }
		[[nodiscard]] uint32_t GetMaxFramebufferSize() const { return s_MaxFramebufferSize; }

	private:
		void Invalidate();
		void AttachExisting();

	private:
		uint32_t m_FrameBufferID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentsSpecification;
		std::vector<Ref<Texture2D>> m_ColorAttachments;

		// Depth attachments always created as textures and for now only supports ONE depth attachment
		FramebufferTextureSpecification m_DepthAttachmentSpecification = ImageFormat::None;
		Ref<Texture2D> m_DepthAttachment = nullptr;

		bool m_AttachExisting = false;

		// 8K DCI which is 256:135 Aspect Ratio and measuring 8192x4320 pixels which is HUGE
		static constexpr uint32_t s_MaxFramebufferSize = 8192;

	};

}