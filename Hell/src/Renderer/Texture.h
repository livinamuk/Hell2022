#pragma once
#include "Header.h"
#include "TextureCompression/DDS_Helpers.h"
#include <stb_image.h>


	class Texture
	{
	public: // Methods
		Texture();
		Texture(std::string filepath);
		Texture(FileInfo fileinfo);
		void BindToUnit0();
		//static void LoadTexture(std::string filename);

	private:
		void ReadFromDisk();
		void LoadToGL();

	public: // fields
		unsigned int ID = 0;
		std::string m_name;
		int m_width, m_height;
		FileInfo m_fileNameInfo;

	private:
		unsigned char* data;
		int nrChannels;
		CMP_Texture				  m_cmpTexture = {};
		bool					  m_isCompressed = false;
	};
