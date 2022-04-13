#pragma once
#include "Header.h"

class Input
{
public: // functions
	static void UpdateMouseInput(GLFWwindow* window);
	static void UpdateKeyboardInput(GLFWwindow* window);
	static void UpdateControllerInput(int controllerIndex);

	

	//static void HandleKeypresses(Game* game);
	static void HandleKeypresses();
	//static void HandleKeydowns();

	static bool LeftMouseDown();
	static bool RightMouseDown();
	static bool LeftMousePressed();
	static bool RightMousePressed();
	static bool ButtonPressed(int controllerIndex, unsigned int keycode);
	static bool ButtonDown(int controllerIndex, unsigned int keycode);

	static bool KeyPressed(unsigned int keycode);
	static bool KeyDown(unsigned int keycode);

public: // variables
	static double s_mouseX;
	static double s_mouseY;
	static int s_storedMouseX;
	static int s_storedMouseY;
	static int s_mouseX_Editor;
	static int s_mouseY_Editor;
	static bool s_showCursor;
	static int s_mouseWheelValue;

	static bool s_showBulletDebug;

	static bool s_leftMouseDown;
	static bool s_rightMouseDown;
	static bool s_leftMousePressed;
	static bool s_rightMousePressed;
	static bool s_leftMouseDownLastFrame;
	static bool s_rightMouseDownLastFrame;

	static bool s_keyPressed[372];
	static bool s_keyDown[372];
	static bool s_keyDownLastFrame[372];

	#define MAX_CONTROLLER_COUNT 4 

	static ControllerState s_controllerStates[MAX_CONTROLLER_COUNT];

	static double m_oldX, m_oldY;		// Old mouse position
	static double m_xoffset, m_yoffset;	// Distance mouse moved during current frame in pixels
	static bool m_disable_MouseLook;
	static int m_disableMouseLookTimer;
	static float m_mmouseSensitivity;

	static bool s_mouseWasMovedThisFrame;
};
