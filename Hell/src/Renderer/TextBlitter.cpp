#include "TextBlitter.h"
#include <algorithm>
#include "Core/CoreGL.h"
#include "Helpers/Util.h"

unsigned int TextBlitter::VAO = 0;
unsigned int TextBlitter::VBO = 0;
unsigned int TextBlitter::currentCharIndex = 0;
float TextBlitter::s_blitTimer = 0;
float TextBlitter::s_blitSpeed = 50;
float TextBlitter::s_waitTimer = 0;
float TextBlitter::s_timeToWait = 2;
bool TextBlitter::s_centerText = true;
std::string TextBlitter::s_CharSheet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-=_+[]{}\\|;:'\".,<>/?`~ ";
std::string TextBlitter::s_textToBlit = "";

void TextBlitter::UpdateBlitter(float deltaTime)
{
	// Main timer
	s_blitTimer += deltaTime * s_blitSpeed;
	currentCharIndex = s_blitTimer;
	currentCharIndex = std::min(currentCharIndex, (unsigned int)s_textToBlit.size());

	if (s_blitSpeed == -1) {
		currentCharIndex = (unsigned int)s_textToBlit.size();
		return;
	}

	if (s_timeToWait == -1)
		return;

	// Wait time
	if (currentCharIndex == (unsigned int)s_textToBlit.size())
		s_waitTimer += deltaTime;
	if (s_waitTimer > s_timeToWait)
		s_textToBlit = "";
}

void TextBlitter::DrawTextBlit(Shader* shader)
{
	if (currentCharIndex == 0)
		return;

	// Have to hard code this coz otherwise the text is the same size in pixels regardless of resolution.
	float screenWidth = 1920;
	float screenHeight = 1080;

	// If you are actually blitting (a non -1 speed), and have already waited long enough, then don't draw shit
	if (s_blitSpeed != -1 && s_waitTimer > s_timeToWait)
		return;

	float charWidth = 16;
	float charHeight = 32;
	float h = charHeight / (screenHeight / 2);
	float w = charWidth / (screenWidth / 2);
	float textureWidth = 766 * 2;
	static float textScale = 0.75f;
	static float lineHeight = 38 / (screenHeight / 2);;
	static float cursor_X = 0;
	static float cursor_Y = 0;
	static float margin_X = -1.00f / textScale;
	static float margin_Y = (1.0f / textScale) - h;

	std::vector<Vertex2D> vertices;


	size_t lastLineBreakIndex = 0;
	//bool centered;

	vertices.clear();

	// Find the length of the first line
	int lengthOfLine = s_textToBlit.find('\n', 0);


	//If there are no lline breaks then it's the whole string
	if (lengthOfLine == -1)
		lengthOfLine = s_textToBlit.size();

	// Center the cursor
	if (s_centerText)
	{
		float centerPosition = 0 - ((lengthOfLine * w) * 0.5f);
		cursor_X = centerPosition;
		cursor_Y = -0.5f;
	}
	// else use a margin
	else
	{
		cursor_X = margin_X;
		cursor_Y = margin_Y;
	}



	for (int i = 0; i < currentCharIndex; i++)
	{
		char character = s_textToBlit[i];
			
		float totalWidth;
		//if (s_textToBlit.find('\n', 0) != -1)
			totalWidth = s_textToBlit.find('\n', lastLineBreakIndex) * w;
		//else
		//	totalWidth = s_textToBlit.length() * w;

		// If there wasn't one, then it's simply the beginning of string
		if (lastLineBreakIndex == -1)
			lastLineBreakIndex = 0;

		// Check for line brak
		if (character == '\n')
		{
			lastLineBreakIndex = i;
			std::string nextLine = s_textToBlit.substr(lastLineBreakIndex + 1, s_textToBlit.find('\n', lastLineBreakIndex + 1) - i);
			lengthOfLine = nextLine.length() - 1;

			if (lengthOfLine == -1)
				lengthOfLine = s_textToBlit.size() - i;

			if (s_centerText)
				cursor_X = 0 - ((lengthOfLine * w) * 0.5f);
			else
				cursor_X = margin_X;

			cursor_Y -= lineHeight;
			continue;
		}

		else 
		{
			int charPos = s_CharSheet.find(character);

			float tex_coord_L = (charWidth / textureWidth) * charPos;
			float tex_coord_R = (charWidth / textureWidth) * (charPos + 1);

			Vertex2D v1 = { glm::vec2(cursor_X, cursor_Y + h), glm::vec2(tex_coord_L, 0) };
			Vertex2D v2 = { glm::vec2(cursor_X, cursor_Y), glm::vec2(tex_coord_L, 1) };
			Vertex2D v3 = { glm::vec2(cursor_X + w, cursor_Y + h), glm::vec2(tex_coord_R, 0) };
			Vertex2D v4 = { glm::vec2(cursor_X + w, cursor_Y), glm::vec2(tex_coord_R, 1) };

			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
			vertices.push_back(v4);

			cursor_X += w;
		}			
	}

	if (VAO == 0)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		std::cout << "Initialized Blitter.\n";
	}
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	if (vertices.size())
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex2D), &vertices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(textScale, textScale, 1));
	shader->setMat4("model", modelMatrix);
	shader->setVec3("colorTint", glm::vec3(1, 1, 1));

	glEnable(GL_CULL_FACE);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
	glBindVertexArray(0);
}

void TextBlitter::TypeText(std::string text, bool centered)
{
	s_blitTimer = 0;
	s_waitTimer = 0;
	s_blitSpeed = 50;
	s_textToBlit = text;
	s_centerText = centered;
	s_timeToWait = 2;
}

void TextBlitter::BlitText(std::string text, bool centered)
{
	currentCharIndex = text.length();
	s_blitTimer = 0;
	s_waitTimer = 0;
	s_blitSpeed = -1;
	s_textToBlit = text;
	s_centerText = centered;
	s_timeToWait = -1;
}

void TextBlitter::ResetBlitter()
{
	currentCharIndex = 0;
	s_blitTimer = 0;
	s_waitTimer = 0;
	s_blitSpeed = -1;
	s_textToBlit = "";
	s_centerText = false;
	s_timeToWait = -1;
}

void TextBlitter::BlitLine(std::string line)
{
	s_textToBlit += line + "\n";
}
