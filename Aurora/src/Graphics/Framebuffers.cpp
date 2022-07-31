#include "Aurorapch.h"
#include "Framebuffers.h"

#include <glad/glad.h>

namespace Aurora {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {

		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampling, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampling), count, outID); // 1. Created Textures
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void CreateRenderBuffer(uint32_t* outID, uint32_t count)
		{
			glCreateRenderbuffers(1, outID);
		}

		static void BindRenderBuffer(bool multisampled, uint32_t id)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multiSampled = samples > 1;

			if (multiSampled)
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			else
			{
				// 2. Allocated Memory for the textures
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr); // In the future it could be something other than GL_UNSIGNED_BYTE therefore a conversion fucntion is needed on the type
				
				// TODO: Filtering and Wrapping Change to use my texture api and this stuff is to set from the framebufferSpecification
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			// 3. Attached the texture to our framebuffer at the proper index
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multiSampled), id, 0);
		}

		static void AttachDepthRenderBuffer(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multiSampled = samples > 1;

			if (multiSampled)
				glNamedRenderbufferStorageMultisample(id, samples, format, width, height);
			else
				// 2. Allocated Memory for the textures
				glNamedRenderbufferStorage(id, format, width, height);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, id);
		}

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			    case FrameBufferTextureFormat::DEPTH24STENCIL8:   return true;
			}

			return false;
		}

		static GLenum GLInternalFormatFromFramebufferTextureFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			    case Aurora::FrameBufferTextureFormat::None:                 return GL_NONE;
			    case Aurora::FrameBufferTextureFormat::RGBA8:                return GL_RGBA8;
			    case Aurora::FrameBufferTextureFormat::RED_INTEGER:          return GL_RED_INTEGER;
			    case Aurora::FrameBufferTextureFormat::DEPTH24STENCIL8:      return GL_DEPTH24_STENCIL8;
			}

			AR_CORE_ASSERT(false, "Unkown texture format");
			return 0;
		}

	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<Framebuffer>(spec);
	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto format : spec.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentsSpecification.emplace_back(format);
			else
				m_DepthAttachmentSpecification = format;
		}

		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		AR_PROFILE_FUNCTION();

		glDeleteFramebuffers(1, &m_BufferID);
		glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteRenderbuffers(1, &m_DepthAttachment);
	}

	void Framebuffer::Invalidate() // When something has changed in the framebuffer and it is no longer valid this function is called
	{
		AR_PROFILE_FUNCTION();

		if (m_BufferID)
		{
			glDeleteFramebuffers(1, &m_BufferID);
			glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteRenderbuffers(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_BufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

		// Attachments
		bool multiSample = m_Specification.Samples > 1;

		if (m_ColorAttachmentsSpecification.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentsSpecification.size());
			Utils::CreateTextures(multiSample, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multiSample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentsSpecification[i].TextureFormat)
				{
				    case FrameBufferTextureFormat::RGBA8:
				    	Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, (int)i);
				    	break;
					case FrameBufferTextureFormat::RED_INTEGER:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, (int)i);
						break;
				}
			}
		}

		if (m_DepthAttachmentSpecification.TextureFormat != FrameBufferTextureFormat::None)
		{
			Utils::CreateRenderBuffer(&m_DepthAttachment, 1);
			Utils::BindRenderBuffer(multiSample, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
			    case FrameBufferTextureFormat::DEPTH24STENCIL8:
			    	Utils::AttachDepthRenderBuffer(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
			    	break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			AR_CORE_ASSERT(m_ColorAttachments.size() <= 4, "For now we only support a max of 4 color attachments");

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

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			AR_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		AR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index can not be more than the available attachments");

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	// This should be changed from taking just an integer back to const void* and figuring the type later
	void Framebuffer::ClearTextureAttachment(uint32_t attachmentIndex, int data) const
	{
		AR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Attachment index can not be more than the available attachments");

		// m_ColorAttachments and m_ColorAttachmentsSpecification are always equal in size the indexing matches perfectly
		auto& spec = m_ColorAttachmentsSpecification[attachmentIndex];

		// TODO: GL_INT should be implied from the spec.Textureformat via a switch static function, also with it is the TODO that is up top on line 59
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::GLInternalFormatFromFramebufferTextureFormat(spec.TextureFormat), GL_INT, &data);
	}

	void Framebuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Framebuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}