//#include "hellpch.h"
#include "Input.h"
#include "keycodes.h"
#include "CoreGL.h"
//#include "Core/CoreImGui.h"
//#include "Game.h"
//#include "Logic/WeaponLogic.h"
#include <algorithm>

double Input::s_mouseX;
double Input::s_mouseY;
int Input::s_storedMouseX;
int Input::s_storedMouseY;
int Input::s_mouseX_Editor;
int Input::s_mouseY_Editor;
bool Input::s_showCursor = false;
bool Input::s_keyPressed[372] = { false };
bool Input::s_keyDown[372] = { false };
bool Input::s_keyDownLastFrame[372] = { false }; 

bool Input::s_leftMouseDown = false;
bool Input::s_rightMouseDown = false;

bool Input::s_leftMousePressed = false;
bool Input::s_rightMousePressed = false;

bool Input::s_leftMouseDownLastFrame = false;
bool Input::s_rightMouseDownLastFrame = false;
bool Input::s_showBulletDebug = false;
int Input::s_mouseWheelValue = 0;

/*
float Input::m_oldX;
float Input::m_oldY;
float Input::m_xoffset;
float Input::m_yoffset;*/


double Input::m_oldX;
double Input::m_oldY;		// Old mouse position
double Input::m_xoffset;
double Input::m_yoffset;	// Distance mouse moved during current frame in pixels
bool Input::m_disable_MouseLook = false;
int Input::m_disableMouseLookTimer = 10;
float Input::m_mmouseSensitivity;	
bool Input::s_mouseWasMovedThisFrame;

ControllerState Input::s_controllerStates[MAX_CONTROLLER_COUNT];

void Input::UpdateMouseInput(GLFWwindow* window)
{
	glfwGetCursorPos(window, &s_mouseX, &s_mouseY);	
	m_xoffset = s_mouseX - m_oldX;
	m_yoffset = s_mouseY - m_oldY;
	m_oldX = s_mouseX;
	m_oldY = s_mouseY;

	// Left mouse down/pressed
	s_leftMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (s_leftMouseDown == GLFW_PRESS && !s_leftMouseDownLastFrame)
		s_leftMousePressed = true;
	else
		s_leftMousePressed = false;
	s_leftMouseDownLastFrame = s_leftMouseDown;

	// Right mouse down/pressed
	s_rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (s_rightMouseDown == GLFW_PRESS && !s_rightMouseDownLastFrame)
		s_rightMousePressed = true;
	else
		s_rightMousePressed = false;
	s_rightMouseDownLastFrame = s_rightMouseDown;
}

void Input::HandleKeypresses()
{
	// Toggle Bullet Debug Drawer
	if (s_keyPressed[HELL_KEY_B])
		s_showBulletDebug = !s_showBulletDebug;

	// Toggle cursor
	if (s_keyPressed[HELL_KEY_M])
		s_showCursor = !s_showCursor;
}

bool Input::LeftMouseDown()
{
	return s_leftMouseDown;
}

bool Input::RightMouseDown()
{
	return s_rightMouseDown;
}

bool Input::LeftMousePressed()
{
	return s_leftMousePressed;
}

bool Input::RightMousePressed()
{
	return s_rightMousePressed;
}

bool Input::ButtonPressed(int controllerIndex, unsigned int keycode)
{
	if (controllerIndex == -1 || controllerIndex >= MAX_CONTROLLER_COUNT)
		return false;

	return s_controllerStates[controllerIndex].buttons_pressed[keycode];
}

bool Input::ButtonDown(int controllerIndex, unsigned int keycode)
{
	if (controllerIndex == -1 || controllerIndex >= MAX_CONTROLLER_COUNT)
		return false;

	return s_controllerStates[controllerIndex].buttons_down[keycode];
}

bool Input::KeyPressed(unsigned int keycode)
{
	return s_keyPressed[keycode];
}

bool Input::KeyDown(unsigned int keycode)
{
	return s_keyDown[keycode];
}

void Input::UpdateKeyboardInput(GLFWwindow* window)
{
	// Left mouse down
	if (Input::s_leftMouseDown)
		Input::s_keyDown[HELL_MOUSE_LEFT] = true;
	else
		Input::s_keyDown[HELL_MOUSE_LEFT] = false;
	// Left mouse pressed
	if (Input::s_keyDown[HELL_MOUSE_LEFT] && !Input::s_keyDownLastFrame[HELL_MOUSE_LEFT])
		Input::s_keyPressed[HELL_MOUSE_LEFT] = true;
	else
		Input::s_keyPressed[HELL_MOUSE_LEFT] = false;

	// Right mouse down
	if (Input::s_rightMouseDown)
		Input::s_keyDown[HELL_MOUSE_RIGHT] = true;
	else
		Input::s_keyDown[HELL_MOUSE_RIGHT] = false;
	// Right mouse pressed
	if (Input::s_keyDown[HELL_MOUSE_RIGHT] && !Input::s_keyDownLastFrame[HELL_MOUSE_RIGHT])
		Input::s_keyPressed[HELL_MOUSE_RIGHT] = true;
	else
		Input::s_keyPressed[HELL_MOUSE_RIGHT] = false;

	// update last frame state
	Input::s_keyDownLastFrame[HELL_MOUSE_LEFT] = Input::s_keyDown[HELL_MOUSE_LEFT];
	Input::s_keyDownLastFrame[HELL_MOUSE_RIGHT] = Input::s_keyDown[HELL_MOUSE_RIGHT];

	// Keyboard
	for (int i = 30; i < 350; i++) {
		// Key down
		if (glfwGetKey(window, i) == GLFW_PRESS)
			Input::s_keyDown[i] = true;
		else
			Input::s_keyDown[i] = false;

		// Key press
		if (Input::s_keyDown[i] && !Input::s_keyDownLastFrame[i])
			Input::s_keyPressed[i] = true;
		else
			Input::s_keyPressed[i] = false;
		Input::s_keyDownLastFrame[i] = Input::s_keyDown[i];
	}
}

void Input::UpdateControllerInput(int controllerIndex)
{
	if (controllerIndex == -1)
		return;
	
	GLFWgamepadstate state;
	int buttonCount;
	int axesCount;


	// Controller 1
	if (glfwGetGamepadState(controllerIndex, &state))
	{
		const unsigned char* buttons = glfwGetJoystickButtons(controllerIndex, &buttonCount);
		for (int i = 0; i < buttonCount - 2; i++)
		{
			int buttonCode = i; // 352 is where the controller button codes start in keycodes.h
			// button down
			if (state.buttons[i] == GLFW_PRESS)
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = false;
			// button press
			if (s_controllerStates[controllerIndex].buttons_down[buttonCode] && !s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode])
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = false;
			s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode] = s_controllerStates[controllerIndex].buttons_down[buttonCode];
		}

		// Get axes
		const float* axes = glfwGetJoystickAxes(controllerIndex, &axesCount);

		// Check Left trigger
		{
			int buttonCode = HELL_PS_4_CONTROLLER_TRIGGER_L;
			if (axes[3] > -0.9f)
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = false;
			// button press
			if (s_controllerStates[controllerIndex].buttons_down[buttonCode] && !s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode])
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = false;
			s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode] = s_controllerStates[controllerIndex].buttons_down[buttonCode];
		}

		// Check Right trigger
		{
			int buttonCode = HELL_PS_4_CONTROLLER_TRIGGER_R;
			if (axes[4] > -0.9f)
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_down[buttonCode] = false;
			// button press
			if (s_controllerStates[controllerIndex].buttons_down[buttonCode] && !s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode])
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = true;
			else
				s_controllerStates[controllerIndex].buttons_pressed[buttonCode] = false;
			s_controllerStates[controllerIndex].buttons_down_last_frame[buttonCode] = s_controllerStates[controllerIndex].buttons_down[buttonCode];
		}

		// Sticks
		s_controllerStates[controllerIndex].left_stick_axis_X = axes[0];
		s_controllerStates[controllerIndex].left_stick_axis_Y = axes[1];
		s_controllerStates[controllerIndex].right_stick_axis_X = axes[2];
		s_controllerStates[controllerIndex].right_stick_axis_Y = axes[5];
	}
}