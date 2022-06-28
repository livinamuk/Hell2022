//#include "hellpch.h"
#include "Input.h"
#include "keycodes.h"
#include "CoreGL.h"
//#include "Core/CoreImGui.h"
//#include "Game.h"
//#include "Logic/WeaponLogic.h"
#include <algorithm>
#include "GameData.h"

#include <iostream>
#include <Windows.h>
#include <vector>
#include <set>

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

std::vector<MouseState> Input::s_mouseStates;
std::vector<KeyboardState> Input::s_keyboardStates;

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






#include <iostream>
#include <Windows.h>
#include <vector>
#include <set>

using namespace std;

const USHORT HID_USAGE_GENERIC_MOUSE = 0x02;
const USHORT HID_USAGE_GENERIC_KEYBOARD = 0x06;

vector<HANDLE> mouseHandles;
vector<HANDLE> keyboardHandles;

int GetHandleIndex(vector<HANDLE>* handleVector, HANDLE handle) {
	for (int i = 0; i < handleVector->size(); i++) {
		if ((*handleVector)[i] == handle) {
			return i;
		}
	}

	handleVector->push_back(handle);
	return handleVector->size() - 1;
}





LRESULT CALLBACK targetWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INPUT) {
		UINT dataSize = 0;
		// First call to get data size
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

		if (dataSize > 0)
		{
			RAWINPUT raw = RAWINPUT();
			// Second call to get the actual data
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &dataSize, sizeof(RAWINPUTHEADER)) == dataSize) 
			{
				// Mice
				if (raw.header.dwType == RIM_TYPEMOUSE) 
				{
					int mouseID = GetHandleIndex(&mouseHandles, raw.header.hDevice);
					if (mouseID >= 4) return 0;

					switch (raw.data.mouse.ulButtons) {
					case RI_MOUSE_LEFT_BUTTON_DOWN: Input::s_mouseStates[mouseID].leftMouseDown = true;	break;
					case RI_MOUSE_LEFT_BUTTON_UP: Input::s_mouseStates[mouseID].leftMouseDown = false; break;
					case RI_MOUSE_RIGHT_BUTTON_DOWN: Input::s_mouseStates[mouseID].rightMouseDown = true; break;
					case RI_MOUSE_RIGHT_BUTTON_UP: Input::s_mouseStates[mouseID].rightMouseDown = false; break;
					}

					// Wheel change values are device-dependent. Check RAWMOUSE docs for details.
					if (raw.data.mouse.usButtonData != 0) {
						//	cout << "MOUSE " << mouseID << ": WHEEL CHANGE " << raw.data.mouse.usButtonData << endl;
					}

					Input::s_mouseStates[mouseID].xoffset += raw.data.mouse.lLastX;
					Input::s_mouseStates[mouseID].yoffset += raw.data.mouse.lLastY;		
				}
				// Keyboard
				else if (raw.header.dwType == RIM_TYPEKEYBOARD) 
				{
					int keyboardID = GetHandleIndex(&keyboardHandles, raw.header.hDevice);
					auto keycode = raw.data.keyboard.VKey;
					if (keyboardID >= 4) return 0;
					
					//std::cout << keycode << "\n";

					switch (raw.data.keyboard.Flags) {
					case RI_KEY_MAKE: Input::s_keyboardStates[keyboardID].keyDown[keycode] = true; break;
					case RI_KEY_BREAK: Input::s_keyboardStates[keyboardID].keyDown[keycode] = false; break;
					}
				}
			}
		}
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool RegisterDeviceOfType(USHORT type, HWND eventWindow) {
	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;
	rid.usUsage = type;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = eventWindow;
	return RegisterRawInputDevices(&rid, 1, sizeof(rid));
}


void Input::Init()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASS windowClass = {};
	windowClass.lpfnWndProc = targetWindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = TEXT("InputWindow");
	if (!RegisterClass(&windowClass)) {
		std::cout << "Failed to register window class\n";
		return;
	}
	HWND eventWindow = CreateWindowEx(0, windowClass.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
	if (!eventWindow) {
		std::cout << "Failed to register window class\n";
		return;
	}
	else
		std::cout << "Dual keyboard init successful\n";
	RegisterDeviceOfType(HID_USAGE_GENERIC_MOUSE, eventWindow);
	RegisterDeviceOfType(HID_USAGE_GENERIC_KEYBOARD, eventWindow);

	// Add support for 4 mice/keyboard
	for (int i = 0; i < 4; i++) {
		s_mouseStates.push_back(MouseState());
		s_keyboardStates.push_back(KeyboardState());
	}
}

void Input::Update()
{
	MSG msg; 
//	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	for (MouseState& state : s_mouseStates) {
		// Left mouse down/pressed
		if (state.leftMouseDown && !state.leftMouseDownLastFrame)
			state.leftMousePressed = true;
		else
			state.leftMousePressed = false;
		state.leftMouseDownLastFrame = state.leftMouseDown;

		// Right mouse down/pressed
		if (state.rightMouseDown && !state.rightMouseDownLastFrame)
			state.rightMousePressed = true;
		else
			state.rightMousePressed = false;
		state.rightMouseDownLastFrame = state.rightMouseDown;
	}

	for (KeyboardState& state : s_keyboardStates) {
		// Key press
		for (int i = 0; i < 350; i++) {
			if (state.keyDown[i] && !state.keyDownLastFrame[i])
				state.keyPressed[i] = true;
			else
				state.keyPressed[i] = false;
			state.keyDownLastFrame[i] = state.keyDown[i];
		}
	}
}


void Input::ResetMouseOffsets()
{
	for (MouseState& state : s_mouseStates) {
		state.xoffset = 0;
		state.yoffset = 0;
	}
}

//ControllerState Input::s_controllerStates[MAX_CONTROLLER_COUNT];

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

int Input::GetMouseYOffset(int index)
{
	if (index < 0 || index >= 4)
		return 0;
	else
		return s_mouseStates[index].xoffset;
}

bool Input::LeftMouseDown()
{
	return s_leftMouseDown;
}

bool Input::LeftMouseDown(int index)
{
	if (index < 0 || index >= 4)
		return false;
	else
		return s_mouseStates[index].leftMouseDown;
}

bool Input::RightMouseDown()
{
	return s_rightMouseDown;
}

bool Input::RightMouseDown(int index)
{
	if (index < 0 || index >= 4)
		return false;
	else
		return s_mouseStates[index].rightMouseDown;
}

bool Input::LeftMousePressed()
{
	return s_leftMousePressed;
}

bool Input::LeftMousePressed(int index)
{
	if (index < 0 || index >=4)
		return false;
	else
		return s_mouseStates[index].leftMousePressed;
}

bool Input::RightMousePressed()
{
	return s_rightMousePressed;
}

bool Input::RightMousePressed(int index)
{
	if (index < 0 || index >= 4)
		return false;
	else
		return s_mouseStates[index].rightMousePressed;
}


int Input::GetMouseXOffset(int index)
{
	if (index < 0 || index >= 4)
		return 0;
	else
		return s_mouseStates[index].yoffset;
}

bool Input::ButtonPressed(int controllerIndex, unsigned int keycode)
{
	if (controllerIndex == -1)// || controllerIndex >= MAX_CONTROLLER_COUNT)
		return false;

	return GameData::s_controllers[controllerIndex].buttons_pressed[keycode];
}

bool Input::ButtonDown(int controllerIndex, unsigned int keycode)
{
	if (controllerIndex == -1 )//|| controllerIndex >= MAX_CONTROLLER_COUNT)
		return false;

	return GameData::s_controllers[controllerIndex].buttons_down[keycode];
}

bool Input::KeyPressed(unsigned int keycode)
{
	return s_keyPressed[keycode];
}

bool Input::KeyDown(unsigned int keycode)
{
	return s_keyDown[keycode];
}

bool Input::KeyDown(int keyboardIndex, int mouseIndex, unsigned int keycode)
{
	// It's a mouse button
	if (keycode == HELL_MOUSE_LEFT && mouseIndex >= 0 && mouseIndex < 4)
		return s_mouseStates[mouseIndex].leftMouseDown;
	else if (keycode == HELL_MOUSE_RIGHT && mouseIndex >= 0 && mouseIndex < 4)
		return s_mouseStates[mouseIndex].rightMouseDown;
	
	// It's a keyboard button
	else if (keyboardIndex >= 0 && keyboardIndex < 4)
		return s_keyboardStates[keyboardIndex].keyDown[keycode];
	// Something else invalid
	else
		return false;
}

bool Input::KeyPressed(int keyboardIndex, int mouseIndex, unsigned int keycode)
{
	// It's a mouse button
	if (keycode == HELL_MOUSE_LEFT && mouseIndex >= 0 && mouseIndex < 4)
		return s_mouseStates[mouseIndex].leftMousePressed;
	else if (keycode == HELL_MOUSE_RIGHT && mouseIndex >= 0 && mouseIndex < 4)
		return s_mouseStates[mouseIndex].rightMousePressed;

	// It's a keyboard button
	else if (keyboardIndex >= 0 && keyboardIndex < 4)
		return s_keyboardStates[keyboardIndex].keyPressed[keycode];
	// Something else invalid
	else
		return false;
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

/*void Input::UpdateControllerInput(int controllerIndex)
{
	
}*/