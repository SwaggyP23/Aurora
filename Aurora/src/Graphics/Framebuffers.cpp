#include "Aurorapch.h"
#include "Framebuffers.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLDepthAttachmentType(ImageFormat format)
		{
			switch (format)
			{
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

		// TODO: Expand on this for all the other ImageFormats!!
		static uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::R8UI:        return 1;
			    case ImageFormat::R16UI:       return 2;
			    case ImageFormat::R32UI:       return 4;
			    case ImageFormat::R32F:        return 4;
			    case ImageFormat::RGB:	       return 3;
			    case ImageFormat::SRGB:        return 3;
			    case ImageFormat::RGBA:        return 4;
			    case ImageFormat::RGBA16F:     return 2 * 4;
			    case ImageFormat::RGBA32F:     return 4 * 4;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			return width * height * GetImageFormatBPP(format);
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
			glCreateRenderbuffers(1, outID);
		}

		inline static void BindRenderBuffer(uint32_t id)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, id);
		}

		static void AttachColorTexture(uint32_t id, uint32_t samples, const FramebufferTextureSpecification& spec, uint32_t width, uint32_t height, uint32_t index, bool fixedSamples)
		{
			bool multiSampled = samples > 1;
			GLenum internalFormat = GLInternalFormatFromAFormat(spec.TextureFormat);
			GLenum format = GLFormatFromAFormat(spec.TextureFormat);
			GLenum dataType = GLDataTypeFromAFormat(spec.TextureFormat);

			if (multiSampled)
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, fixedSamples ? GL_TRUE : GL_FALSE);
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);
				
				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_WRAP_R, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GLWrapTypeFromTextureWrap(spec.WrapMode));
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multiSampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, uint32_t samples, const FramebufferTextureSpecification& spec, uint32_t width, uint32_t height)
		{
			bool multiSampled = samples > 1;
			GLenum internalFormat = GLInternalFormatFromAFormat(spec.TextureFormat);
			GLenum format = GLFormatFromAFormat(spec.TextureFormat);
			GLenum dataType = GLDataTypeFromAFormat(spec.TextureFormat);

			if (multiSampled)
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);

				glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GLFilterTypeFromTextureFilter(spec.FilterMode, false));
				glTextureParameteri(id, GL_TEXTURE_WRAP_R, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_S, GLWrapTypeFromTextureWrap(spec.WrapMode));
				glTextureParameteri(id, GL_TEXTURE_WRAP_T, GLWrapTypeFromTextureWrap(spec.WrapMode));
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GLDepthAttachmentType(spec.TextureFormat), TextureTarget(multiSampled), id, 0);
		}

		static void AttachDepthRenderBuffer(uint32_t id, uint32_t samples, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
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

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, id);
		}

	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<Framebuffer>(spec);
	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (const auto& format : spec.AttachmentsSpecification.Attachments)
		{
			if (Utils::IsDepthFormat(format.TextureFormat))
				m_DepthAttachmentSpecification = format;
			else
				m_ColorAttachmentsSpecification.emplace_back(format);
		}

		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_FrameBufferID);
		glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteRenderbuffers(1, &m_DepthAttachment);
	}

	void Framebuffer::Invalidate()
	{
		if (m_FrameBufferID)
		{
			glDeleteFramebuffers(1, &m_FrameBufferID);
			glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteRenderbuffers(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_FrameBufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

		// Attachments
		const bool multiSample = m_Specification.Samples > 1;

		// Color attachments
		if (m_ColorAttachmentsSpecification.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentsSpecification.size());
			Utils::CreateTextures(multiSample, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multiSample, m_ColorAttachments[i]);

				Utils::AttachColorTexture(m_ColorAttachments[i],
					m_Specification.Samples,
					m_ColorAttachmentsSpecification[i],
					m_Specification.Width,
					m_Specification.Height,
					(uint32_t)i,
					m_Specification.DepthAttachmentAsTexture ? false : true);
			}
		}

		// Depth attachments
		if (m_DepthAttachmentSpecification.TextureFormat != ImageFormat::None)
		{
			if (m_Specification.DepthAttachmentAsTexture)
			{
				Utils::CreateTextures(multiSample, &m_DepthAttachment, 1);
				Utils::BindTexture(multiSample, m_DepthAttachment);

				Utils::AttachDepthTexture(m_DepthAttachment, 
					m_Specification.Samples, 
					m_DepthAttachmentSpecification,
					m_Specification.Width,
					m_Specification.Height);
			}
			else
			{
				GLenum internalFormat = Utils::GLInternalFormatFromAFormat(m_DepthAttachmentSpecification.TextureFormat);
				GLenum attachmentType = Utils::GLDepthAttachmentType(m_DepthAttachmentSpecification.TextureFormat);

				Utils::CreateRenderBuffer(&m_DepthAttachment, 1);
				Utils::BindRenderBuffer(m_DepthAttachment);
				Utils::AttachDepthRenderBuffer(m_DepthAttachment,
					m_Specification.Samples,
					internalFormat,
					attachmentType,
					m_Specification.Width,
					m_Specification.Height);
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			AR_CORE_ASSERT(m_ColorAttachments.size() <= 4, "For now we only support a max of 4 color attachments");

			// TODO: For now we only support 4 draw buffer however that could be exapanded since we could support
			// up to 8 attachments!
			// This is us telling OpenGL that we want to draw to 4 different attachments
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
		}
		else if(m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		AR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Incomplete Frambuffer!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Blit(uint32_t src, uint32_t dst, uint32_t srcWidth, uint32_t srcHeight, uint32_t srcAttachment, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstAttachment)
	{
		glNamedFramebufferReadBuffer(src, GL_COLOR_ATTACHMENT0 + srcAttachment);
		glNamedFramebufferDrawBuffer(dst, GL_COLOR_ATTACHMENT0 + dstAttachment);
		glBlitNamedFramebuffer(src, dst, 0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		constexpr uint32_t s_MaxFramebufferSize = 8192;

		if (!m_Specification.Resizable)
			return;

		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			AR_CORE_WARN_TAG("Framebuffer", "Attempted to rezize framebuffer to: {0} - {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void Framebuffer::GetColorAttachmentData(void* pixels, uint32_t attachmentIndex)
	{
		AR_CORE_ASSERT(m_Specification.Samples == 1, "Cant get the pixels of a Multisampled texture!");

		GLenum format = Utils::GLFormatFromAFormat(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat);
		GLenum type = Utils::GLDataTypeFromAFormat(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat);

		uint32_t buffSize = Utils::GetImageMemorySize(m_ColorAttachmentsSpecification[attachmentIndex].TextureFormat, m_Specification.Width, m_Specification.Height);
		glGetTextureImage(m_ColorAttachments[attachmentIndex], 0, format, type, buffSize, pixels);
	}

	void Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y, void* data)
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

		uint32_t textureID = m_ColorAttachments[attachmentIndex];
		glClearTexImage(textureID, 0, Utils::GLFormatFromAFormat(spec.TextureFormat), Utils::GLDataTypeFromAFormat(spec.TextureFormat), data);
	}

	void Framebuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);

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