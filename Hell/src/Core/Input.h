#pragma once
#include "Header.h"

struct MouseState {
	bool detected = false;
	bool leftMouseDown = false;
	bool rightMouseDown = false;
	bool leftMousePressed = false;
	bool rightMousePressed = false;
	bool leftMouseDownLastFrame = false;
	bool rightMouseDownLastFrame = false;
	//double oldX, oldY;			// Old mouse position
	double xoffset = 0;
	double yoffset = 0;	// Distance mouse moved during current frame in pixels
};

struct KeyboardState {
	bool keyPressed[372];
	bool keyDown[372];
	bool keyDownLastFrame[372];
};

class Input
{
public: // functions
	
	static void Init();
	static void Update();
	static void ResetMouseOffsets();




	static void UpdateMouseInput(GLFWwindow* window);
	static void UpdateKeyboardInput(GLFWwindow* window);
	//static void UpdateControllerInput(int controllerIndex);
	

	//static void HandleKeypresses(Game* game);
	static void HandleKeypresses();
	//static void HandleKeydowns();

	static bool LeftMouseDown(int index);
	static bool RightMouseDown(int index);
	static bool LeftMousePressed(int index);
	static bool RightMousePressed(int index);
	static int GetMouseXOffset(int index);
	static int GetMouseYOffset(int index);

	static bool LeftMouseDown();
	static bool RightMouseDown();
	static bool LeftMousePressed();
	static bool RightMousePressed();
	static bool ButtonPressed(int controllerIndex, unsigned int keycode);
	static bool ButtonDown(int controllerIndex, unsigned int keycode);


	static bool KeyPressed(int keyboardIndex, int mouseIndex, unsigned int keycode);
	static bool KeyDown(int keyboardIndex, int mouseIndex, unsigned int keycode);


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

	//#define MAX_CONTROLLER_COUNT 4 

	//static ControllerState s_controllerStates[MAX_CONTROLLER_COUNT];

	static double m_oldX, m_oldY;		// Old mouse position
	static double m_xoffset, m_yoffset;	// Distance mouse moved during current frame in pixels
	static bool m_disable_MouseLook;
	static int m_disableMouseLookTimer;
	static float m_mmouseSensitivity;

	static bool s_mouseWasMovedThisFrame;

	static std::vector<MouseState> s_mouseStates;
	static std::vector<KeyboardState> s_keyboardStates;
};
