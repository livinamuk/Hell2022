#pragma once
#include "Renderer/Shader.h"
#include "Header.h"

class TextBlitter
{
public: //methods
	static void DrawTextBlit(Shader* shader);
	static void UpdateBlitter(float deltaTime);
	static void ResetBlitter();
	static void TypeText(std::string text, bool centered);
	static void BlitText(std::string text, bool centered);
	
	static void BlitLine(std::string line);

private: // methods


public: // fields
	static unsigned int VAO, VBO;
	static unsigned int currentCharIndex;
	static std::string s_textToBlit;
	static float s_blitTimer;
	static float s_blitSpeed;
	static float s_waitTimer;
	static float s_timeToWait;
	static std::string s_CharSheet;
	static bool s_centerText;
};