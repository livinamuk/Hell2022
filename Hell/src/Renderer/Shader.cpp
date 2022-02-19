#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Shader::Shader()
{
}

Shader::Shader(std::string vs, std::string fs)
{
	this->vertexPath = vs;
	this->fragmentPath = fs;
	this->geomPath = "NONE";
	this->ID = LoadShader();
}

Shader::Shader( std::string vs, std::string fs, std::string gs)
{
	this->vertexPath = vs;
	this->fragmentPath = fs;
	this->geomPath = gs;
	this->ID = LoadShader();
}

Shader::~Shader()
{
}

int Shader::LoadShader()
{
	std::string vertSource = ReadFile("res/shaders/" + vertexPath);
	std::string fragSource = ReadFile("res/shaders/" + fragmentPath);
	std::string geomSource = ReadFile("res/shaders/" + geomPath);
	const char* vertexShaderSource = vertSource.c_str();
	const char* fragmentShaderSource = fragSource.c_str();
	const char* geometryShaderSource = geomSource.c_str();

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "FAILED TO COMPILE VERTEX SHADER: " << vertexPath << "\n" << infoLog << std::endl;
	}

	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "FAILED TO COMPILE FRAGMENT SHADER: " << fragmentPath << "\n" << infoLog << std::endl;
	}

	// GEOMETRY SHADER
	int geometryShader = -1;
	if (geomPath != "NONE")
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
		glCompileShader(geometryShader);

		// check for shader compile errors
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			std::cout << "FAILED TO COMPILE GEOMETRY SHADER: " << geomPath << "\n" << infoLog << std::endl;
		}
		//else
		//	std::cout << "GEOMETRY SHADER COMPILED\n";
	}

	// link shaders

	int ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	if (geomPath != "NONE") glAttachShader(ID, geometryShader);
	glLinkProgram(ID);

	// check for linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "FAILED TO LINK SHADER: " << "remember you aren't storing the name anymore lol " << "\n" << infoLog << std::endl;
		return 0;
	}
	//else
	//	std::cout << "SHADER LINKING SUCCESSFUL: " << name << "\n";
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (geomPath != "NONE")
		glDeleteShader(geometryShader);

	return ID;
}

void Shader::ReloadShader()
{
	int newShaderID = LoadShader();
	std::cout << "old id: " << ID << "\n";
	std::cout << "new id: " << newShaderID << "\n";

	if (newShaderID) {
		glUseProgram(0);
		glDeleteProgram(this->ID);
		this->ID = newShaderID;
	}
}

std::string Shader::ReadFile(std::string filepath)
{
	std::string line;
	std::ifstream stream(filepath);
	std::stringstream ss;

	while (getline(stream, line))
		ss << line << "\n";

	return ss.str();
}