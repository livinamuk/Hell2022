#pragma once
#include "Header.h"

class Controller 
{	
public: // enums
	enum class Type { UNKNOWN_TYPE, PS4, XBOX };
	enum class StickMode { AIMING, MOVEMENT };

public: // methods
	Controller();
	void UpdateInput();

public: // fields
	Type m_type;
	int m_glfwIndex; // glfw index
	std::vector<bool> buttons_down;
	std::vector<bool> buttons_down_last_frame;
	std::vector<bool> buttons_pressed;
	float left_stick_axis_X = 0;
	float left_stick_axis_Y = 0;
	float right_stick_axis_X = 0;
	float right_stick_axis_Y = 0;
	float fifth_axis = 0;
	float sixth_axis = 0;
	int axesCount = 0;
};