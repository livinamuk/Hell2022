#include "Texture.h"


	void freeCMPTexture(CMP_Texture* t) {
		free(t->pData);
	}

	constexpr uint32_t GL_COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
	constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
	constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
	constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

	uint32_t cmpToOpenGlFormat(CMP_FORMAT format)
	{
		if (format == CMP_FORMAT_DXT1)
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		else if (format == CMP_FORMAT_DXT3)
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else if (format == CMP_FORMAT_DXT5)
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		else
			return 0xFFFFFFFF;
	}

	Texture::Texture()
	{
	}


	Texture::Texture(std::string filepath)
	{
		if (!FileExists(filepath)) {
			std::cout << filepath << " does not exist.\n";
			return;
		}

		std::string filename = filepath.substr(filepath.rfind("/") + 1);
		filename = filename.substr(0, filename.length() - 4);					// isolate name
		std::string filetype = filepath.substr(filepath.length() - 3);			// isolate filetype	
		std::string directory = filepath.substr(0, filepath.rfind("/") + 1);	// isolate direcetory

		FileInfo info;
		info.fullpath = filepath;
		info.filename = filename;
		info.filetype = filetype;
		info.directory = directory;

		m_name = info.filename + '.' + info.filetype;
		m_fileNameInfo = info;	
		
		ReadFromDisk();
		LoadToGL();
	}

	Texture::Texture(FileInfo info)
	{
		m_name = info.filename + '.' + info.filetype;
		m_fileNameInfo = info;

		if (info.filetype == "dds")
			m_isCompressed = true;
		else
			m_isCompressed = false;

		ReadFromDisk();
		LoadToGL();
	}

	void Texture::BindToUnit0()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ID);
	}


	void Texture::ReadFromDisk()
	{
		// compressed
		if (m_isCompressed)
			LoadDDSFile(m_fileNameInfo.fullpath.c_str(), m_cmpTexture);
		// uncompressed
		else {
			stbi_set_flip_vertically_on_load(false);
			data = stbi_load(m_fileNameInfo.fullpath.c_str(), &m_width, &m_height, &nrChannels, 0);
		}
	}

	void Texture::LoadToGL()
	{
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if (!m_isCompressed)
		{
			GLint format = GL_RGB;

			if (nrChannels == 4)
				format = GL_RGBA;
			if (nrChannels == 1)
				format = GL_RED;

			// Generate texture
			if (data) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				//std::cout << "Loaded texture: " << m_name << "\n";
			}
			else
				std::cout << "Failed to load texture: " << m_name << "\n";

			stbi_image_free(data);
		}

		else
		{
			uint32_t glFormat = cmpToOpenGlFormat(m_cmpTexture.format);

			unsigned int blockSize = (glFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

			if (glFormat != 0xFFFFFFFF)
			{
				uint32_t width = m_cmpTexture.dwWidth;
				uint32_t height = m_cmpTexture.dwHeight;
				uint32_t size1 = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
				uint32_t size2 = m_cmpTexture.dwDataSize;

				glCompressedTexImage2D(GL_TEXTURE_2D, 0, glFormat, m_cmpTexture.dwWidth, m_cmpTexture.dwHeight, 0, size2, m_cmpTexture.pData);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
				glGenerateMipmap(GL_TEXTURE_2D);

				freeCMPTexture(&m_cmpTexture);
				m_cmpTexture = {};
				std::cout << "Loaded compressed texture: " << m_name << "\n";
			}
			else
				std::cout << "Failed to load compressed texture: " << m_name << "\n";
		}
	}
