#include "Aurorapch.h"
#include "Framebuffers.h"

#include "Renderer/Renderer.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLAttachmentType(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::R8I:                      return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R8UI:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R16I:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R16UI:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R32I:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R32UI:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::R32F:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RG8:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RG16F:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RG32F:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RGB:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RGBA:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RGBA16F:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::RGBA32F:					return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::SRGB:						return GL_COLOR_ATTACHMENT0;
			    case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH_STENCIL_ATTACHMENT;
			    case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH_STENCIL_ATTACHMENT;
			    case ImageFormat::DEPTH32F:					return GL_DEPTH_ATTACHMENT;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLDataTypeFromAFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8I:                      return GL_BYTE;
				case ImageFormat::R8UI:						return GL_UNSIGNED_BYTE;
				case ImageFormat::R16I:						return GL_SHORT;
				case ImageFormat::R16UI:					return GL_UNSIGNED_SHORT;
				case ImageFormat::R32I:						return GL_INT;
				case ImageFormat::R32UI:					return GL_UNSIGNED_INT;
				case ImageFormat::R32F:						return GL_FLOAT;
				case ImageFormat::RG8:						return GL_UNSIGNED_BYTE;
				case ImageFormat::RG16F:					return GL_FLOAT;
				case ImageFormat::RG32F:					return GL_FLOAT;
			    case ImageFormat::RGB:						return GL_UNSIGNED_BYTE;
			    case ImageFormat::RGBA:						return GL_UNSIGNED_BYTE;
			    case ImageFormat::RGBA16F:					return GL_FLOAT;
			    case ImageFormat::RGBA32F:					return GL_FLOAT;
			    case ImageFormat::SRGB:						return GL_UNSIGNED_BYTE;
				case ImageFormat::DEPTH32FSTENCIL8UINT:     return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			    case ImageFormat::DEPTH24STENCIL8:			return GL_UNSIGNED_INT_24_8;
				case ImageFormat::DEPTH32F:					return GL_FLOAT;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::DEPTH32FSTENCIL8UINT: return true;
			    case ImageFormat::DEPTH24STENCIL8:		return true;
			    case ImageFormat::DEPTH32F:				return true;
			}

			return false;
		}

		static GLenum GLFormatFromAFormat(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::R8I:						return GL_RED_INTEGER;
			    case ImageFormat::R8UI:						return GL_RED_INTEGER;
				case ImageFormat::R16I:						return GL_RED_INTEGER;
				case ImageFormat::R16UI:					return GL_RED_INTEGER;
			    case ImageFormat::R32I:						return GL_RED_INTEGER;
			    case ImageFormat::R32UI:					return GL_RED_INTEGER;
				case ImageFormat::R32F:						return GL_RED;
				case ImageFormat::RG8:						return GL_RG_INTEGER;
				case ImageFormat::RG16F:					return GL_RG;
				case ImageFormat::RG32F:					return GL_RG;
			    case ImageFormat::RGB:						return GL_RGB;
			    case ImageFormat::RGBA:						return GL_RGBA;
			    case ImageFormat::RGBA16F:					return GL_RGBA;
			    case ImageFormat::RGBA32F:					return GL_RGBA;
			    case ImageFormat::SRGB:						return GL_RGB;
				case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH_STENCIL;
				case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH_STENCIL;
				case ImageFormat::DEPTH32F:					return GL_DEPTH_COMPONENT;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLInternalFormatFromAFormat(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::None:						return GL_NONE;
			    case ImageFormat::R8I:						return GL_R8I;
			    case ImageFormat::R8UI:						return GL_R8UI;
			    case ImageFormat::R16I:						return GL_R16I;
			    case ImageFormat::R16UI:					return GL_R16UI;
			    case ImageFormat::R32I:						return GL_R32I;
			    case ImageFormat::R32UI:					return GL_R32UI;
			    case ImageFormat::R32F:						return GL_R32F;
			    case ImageFormat::RG8:						return GL_RG8;
			    case ImageFormat::RG16F:					return GL_RG16F;
			    case ImageFormat::RG32F:					return GL_RG32F;
			    case ImageFormat::RGB:						return GL_RGB8;
			    case ImageFormat::RGBA:						return GL_RGBA8;
			    case ImageFormat::RGBA16F:					return GL_RGBA16F;
			    case ImageFormat::RGBA32F:					return GL_RGBA32F;
			    case ImageFormat::SRGB:						return GL_SRGB8;
			    case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH32F_STENCIL8;
			    case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH24_STENCIL8;
			    case ImageFormat::DEPTH32F:					return GL_DEPTH_COMPONENT32F;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLFilterTypeFromTextureFilter(TextureFilter type, bool hasMipmap)
		{
			if (type == TextureFilter::None)                        return GL_NONE;
			else if (type == TextureFilter::Nearest && hasMipmap)   return GL_NEAREST_MIPMAP_NEAREST;
			else if (type == TextureFilter::Linear && hasMipmap)    return GL_LINEAR_MIPMAP_LINEAR;
			else if (type == TextureFilter::Nearest && !hasMipmap)  return GL_NEAREST;
			else if (type == TextureFilter::Linear && !hasMipmap)   return GL_LINEAR;

			AR_CORE_ASSERT(false, "Unknown Texture Filter!");
			return 0;
		}

		static GLenum GLWrapTypeFromTextureWrap(TextureWrap type)
		{
			switch (type)
			{
			    case TextureWrap::None:               return GL_NONE;
			    case TextureWrap::Repeat:             return GL_REPEAT;
			    case TextureWrap::Clamp:              return GL_CLAMP_TO_EDGE;
			}

			AR_CORE_ASSERT(false, "Unknown Texture Wrap Mode!");
			return 0;
		}

		inline static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		inline static void CreateTextures(bool multisampling, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampling), count, outID);
		}

		inline static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		inline static void CreateRenderBuffer(uint32_t* outID, uint32_t count)
		{
			glCreateRenderbuffers(count, outID);
		}

		inline static void BindRenderBuffer(uint32_t id)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, id);
		}

		static void AttachColorTexture(uint32_t fboID, uint32_t id, uint32_t samples, const FramebufferTextureSpecification& spec, uint32_t width, uint32_t height, uint32_t index)
		{
			bool multiSampled = samples > 1;
			GLenum internalFormat = GLInternalFormatFromAFormat(spec.TextureFormat);
			GLenum format = GLFormatFromAFormat(spec.TextureFormat);
			GLenum dataType = GLDataTypeFromAFormat(spec.TextureFormat);

			if (multiSampled)
			{
				glTextureStorage2DMultisample(id, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTextureStorage2D(id, 1, internalFormat, width, height);
				
				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_WRAP_R, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GLWrapTypeFromTextureWrap(spec.WrapMode));
			}

			glNamedFramebufferTexture(fboID, GL_COLOR_ATTACHMENT0 + index, id, 0);
		}

		static void AttachDepthTexture(uint32_t fboID, uint32_t id, uint32_t samples, const FramebufferTextureSpecification& spec, uint32_t width, uint32_t height)
		{
			bool multiSampled = samples > 1;
			GLenum internalFormat = GLInternalFormatFromAFormat(spec.TextureFormat);
			GLenum format = GLFormatFromAFormat(spec.TextureFormat);
			GLenum dataType = GLDataTypeFromAFormat(spec.TextureFormat);

			if (multiSampled)
			{
				glTextureStorage2DMultisample(id, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTextureStorage2D(id, 1, internalFormat, width, height);

				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_WRAP_R, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GLWrapTypeFromTextureWrap(spec.WrapMode));
			}

			glNamedFramebufferTexture(fboID, GLAttachmentType(spec.TextureFormat), id, 0);
		}

		static void AttachDepthRenderBuffer(uint32_t fboID, uint32_t id, uint32_t samples, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multiSampled = samples > 1;

			if (multiSampled)
			{
				glNamedRenderbufferStorageMultisample(id, samples, internalFormat, width, height);
			}
			else
			{
				glNamedRenderbufferStorage(id, internalFormat, width, height);
			}

			glNamedFramebufferRenderbuffer(fboID, attachmentType, GL_RENDERBUFFER, id);
		}

	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<Framebuffer>(spec);
	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		// If no existing images create ones
		if (spec.ExistingImages.empty())
		{
			for (const auto& format : spec.AttachmentsSpecification.Attachments)
			{
				if (Utils::IsDepthFormat(format.TextureFormat))
					m_DepthAttachmentSpecification = format;
				else
					m_ColorAttachmentsSpecification.emplace_back(format);
			}

			Invalidate();

			m_AttachExisting = false;
		}
		else
		{
			AR_CORE_CHECK(spec.ExistingImages.size() == spec.AttachmentsSpecification.Attachments.size());

			uint32_t attachmentIndex = 0;
			for (const auto& format : spec.AttachmentsSpecification.Attachments)
			{
				if (Utils::IsDepthFormat(format.TextureFormat))
				{
					m_DepthAttachmentSpecification = format;
					AR_CORE_ASSERT(spec.ExistingImages.find(attachmentIndex) != spec.ExistingImages.end());
					m_DepthAttachment = spec.ExistingImages.find(attachmentIndex)->second;
				}
				else
				{
					m_ColorAttachmentsSpecification.emplace_back(format);
					AR_CORE_ASSERT(spec.ExistingImages.find(attachmentIndex) != spec.ExistingImages.end());
					m_ColorAttachments.emplace_back(spec.ExistingImages.find(attachmentIndex)->second);
				}

				++attachmentIndex;
			}

			m_AttachExisting = true;

			AttachExisting();
		}
	}

	Framebuffer::~Framebuffer()
	{
		// Delete FBO
		glDeleteFramebuffers(1, &m_FrameBufferID);

		// Delete Attachments ONLY if they are NOT existing attachments. Since if they are already existing, their creator should delete them!
		if (!m_AttachExisting)
		{
			for (const auto& texture : m_ColorAttachments)
			{
				uint32_t colorID = texture->GetTextureID();
				glDeleteTextures(1, &colorID);
			}

			if (m_DepthAttachment)
			{
				uint32_t depthID = m_DepthAttachment->GetTextureID();
				glDeleteTextures(1, &depthID);
			}
		}
	}

	void Framebuffer::Invalidate()
	{
		AR_CORE_ASSERT(!m_AttachExisting);

		if (m_FrameBufferID)
		{
			// Delete FBO
			glDeleteFramebuffers(1, &m_FrameBufferID);
			m_FrameBufferID = 0;

			// Delete Attachments ONLY if they are NOT existing attachments. Since if they are already existing, their creator should delete them!
			if (!m_AttachExisting)
			{
				for (const auto& texture : m_ColorAttachments)
				{
					uint32_t colorID = texture->GetTextureID();
					glDeleteTextures(1, &colorID);
				}

				if (m_DepthAttachment)
				{
					uint32_t depthID = m_DepthAttachment->GetTextureID();
					glDeleteTextures(1, &depthID);
				}

				// Reset all
				m_ColorAttachments.clear();
				m_DepthAttachment = nullptr;
			}
		}

		// No need to bind after creation because of DSA
		glCreateFramebuffers(1, &m_FrameBufferID);

		// Attachments
		const bool multiSample = m_Specification.Samples > 1;

		// Color attachments
		if (m_ColorAttachmentsSpecification.size())
		{
			std::vector<uint32_t> colorAttachmentIDs;
			colorAttachmentIDs.resize(m_ColorAttachmentsSpecification.size());
			m_ColorAttachments.resize(m_ColorAttachmentsSpecification.size());
			Utils::CreateTextures(multiSample, colorAttachmentIDs.data(), (uint32_t)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::AttachColorTexture(
					m_FrameBufferID,
					colorAttachmentIDs[i],
					m_Specification.Samples,
					m_ColorAttachmentsSpecification[i],
					m_Specification.Width,
					m_Specification.Height,
					(uint32_t)i);

				// Create an api wrapper over the id
				m_ColorAttachments[i] = Texture2D::Create(colorAttachmentIDs[i], m_Specification.Width, m_Specification.Height, m_ColorAttachmentsSpecification[i].TextureFormat);
			}
		}

		// Depth attachments
		if (m_DepthAttachmentSpecification.TextureFormat != ImageFormat::None)
		{
			uint32_t depthAttachmentID = 0;
			Utils::CreateTextures(multiSample, &depthAttachmentID, 1);

			Utils::AttachDepthTexture(
				m_FrameBufferID,
				depthAttachmentID,
				m_Specification.Samples,
				m_DepthAttachmentSpecification,
				m_Specification.Width,
				m_Specification.Height);

			// Create an api wrapper over the id
			m_DepthAttachment = Texture2D::Create(depthAttachmentID, m_Specification.Width, m_Specification.Height, m_DepthAttachmentSpecification.TextureFormat);
		}

		if (m_ColorAttachments.size() > 0)
		{
			// Number of color attachments should not exceed the maximum amount of draw buffers supported by the implementation!
			uint32_t numOfDrawBuffers = (uint32_t)m_ColorAttachments.size();
			AR_CORE_CHECK(numOfDrawBuffers <= Renderer::GetRendererCapabilities().MaxDrawBuffers);

			std::vector<GLenum> buffers;
			buffers.reserve(numOfDrawBuffers);

			for (uint32_t i = 0; i < numOfDrawBuffers; i++)
				buffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);

			glNamedFramebufferDrawBuffers(m_FrameBufferID, (GLsizei)numOfDrawBuffers, &buffers[0]);
		}
		else
		{
			// Only depth-pass
			glNamedFramebufferDrawBuffer(m_FrameBufferID, GL_NONE);
		}

		AR_CORE_ASSERT(glCheckNamedFramebufferStatus(m_FrameBufferID, GL_FRAMEBUFFER), "Incomplete Framebuffer!");
	}

	void Framebuffer::AttachExisting()
	{
		AR_CORE_ASSERT(m_AttachExisting);

		if (m_FrameBufferID)
		{
			glDeleteFramebuffers(1, &m_FrameBufferID);
			m_FrameBufferID = 0;

			// Should not clear the attachments here since they are just references to already created textures!
		}

		glCreateFramebuffers(1, &m_FrameBufferID);

		const bool multiSample = m_Specification.Samples > 1;

		// Attach color attachments
		for (size_t i = 0; i < m_ColorAttachments.size(); i++)
		{
			glNamedFramebufferTexture(
				m_FrameBufferID,
				(GLenum)(GL_COLOR_ATTACHMENT0 + i),
				m_ColorAttachments[i]->GetTextureID(),
				(int)0);
		}

		// Attach depth attachment
		if (m_DepthAttachmentSpecification.TextureFormat != ImageFormat::None)
		{
			glNamedFramebufferTexture(
				m_FrameBufferID,
				Utils::GLAttachmentType(m_DepthAttachmentSpecification.TextureFormat),
				m_DepthAttachment->GetTextureID(),
				0);
		}

		if (m_ColorAttachments.size() > 0)
		{
			uint32_t numDrawBuffer = (uint32_t)m_ColorAttachments.size();
			AR_CORE_CHECK(numDrawBuffer <= Renderer::GetRendererCapabilities().MaxDrawBuffers);

			std::vector<GLenum> buffers;
			buffers.reserve(numDrawBuffer);

			for (uint32_t i = 0; i < numDrawBuffer; i++)
				buffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);

			glNamedFramebufferDrawBuffers(m_FrameBufferID, numDrawBuffer, &buffers[0]);
		}
		else
		{
			// Only depth-pass
			glNamedFramebufferDrawBuffer(m_FrameBufferID, GL_NONE);
		}

		AR_CORE_ASSERT(glCheckNamedFramebufferStatus(m_FrameBufferID, GL_FRAMEBUFFER), "Incomplete Framebuffer!");
	}

	void Framebuffer::Blit(uint32_t src, uint32_t dst, uint32_t srcWidth, uint32_t srcHeight, uint32_t srcAttachment, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstAttachment, bool depth)
	{
		glNamedFramebufferReadBuffer(src, GL_COLOR_ATTACHMENT0 + srcAttachment);
		glNamedFramebufferDrawBuffer(dst, GL_COLOR_ATTACHMENT0 + dstAttachment);
		glBlitNamedFramebuffer(src, dst, 0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_COLOR_BUFFER_BIT | (depth ? GL_DEPTH_BUFFER_BIT : 0), GL_NEAREST);
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (!m_Specification.Resizable)
			return;

		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			AR_CORE_WARN_TAG("Framebuffer", "Attempted to rezize framebuffer to: {0} - {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		if (m_AttachExisting)
			AttachExisting();
		else
			Invalidate();
	}

	void Framebuffer::GetColorAttachmentData(void* pixels, uint32_t attachmentIndex)
	{
		AR_CORE_ASSERT(m_Specification.Samples == 1, "Cant get the pixels of a Multisampled texture!");

		GLenum format = Utils::GLFormatFromAFormat(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat);
		GLenum type = Utils::GLDataTypeFromAFormat(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat);

		uint32_t buffSize = Utils::GetImageMemorySize(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat, m_Specification.Width, m_Specification.Height);
		glGetTextureImage(m_ColorAttachments[attachmentIndex]->GetTextureID(), 0, format, type, buffSize, pixels);
	}

	void Framebuffer::ReadPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, void* data)
	{
		AR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index can not be more than the available attachments");

		const auto& spec = m_ColorAttachmentsSpecification[attachmentIndex];

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		glReadPixels(x, y, 1, 1, Utils::GLFormatFromAFormat(spec.TextureFormat), Utils::GLDataTypeFromAFormat(spec.TextureFormat), data);
	}

	void Framebuffer::ClearTextureAttachment(uint32_t attachmentIndex, const void* data)
	{
		AR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index can not be more than the available attachments");

		// m_ColorAttachments and m_ColorAttachmentsSpecification are always equal in size the indexing matches perfectly
		const auto& spec = m_ColorAttachmentsSpecification[attachmentIndex];

		uint32_t textureID = m_ColorAttachments[attachmentIndex]->GetTextureID();
		glClearTexImage(textureID, 0, Utils::GLFormatFromAFormat(spec.TextureFormat), Utils::GLDataTypeFromAFormat(spec.TextureFormat), data);
	}

	void Framebuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);

		if (!m_Specification.ClearOnBind)
			return;

		const glm::vec4& clearColor = m_Specification.ClearColor;
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		bool colorBuffer = m_ColorAttachments.size() ? true : false;
		glClear((colorBuffer ? GL_COLOR_BUFFER_BIT : 0) | (m_DepthAttachment ? GL_DEPTH_BUFFER_BIT : 0));
	}

	void Framebuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}