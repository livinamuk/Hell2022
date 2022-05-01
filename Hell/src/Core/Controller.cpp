#include "Controller.h"

Controller::Controller()
{
	// Resize the buttons vector, 22 should be enough, final 2 buttons are reserved for triggers
	buttons_down.resize(22);
	buttons_down_last_frame.resize(22);
	buttons_pressed.resize(22);

	// Wipe the button states to false
	std::fill(buttons_down.begin(), buttons_down.end(), false);
	std::fill(buttons_down_last_frame.begin(), buttons_down_last_frame.end(), false);
	std::fill(buttons_pressed.begin(), buttons_pressed.end(), false);
}

void Controller::UpdateInput()
{
	if (!glfwJoystickIsGamepad(m_glfwIndex))
		return;

	int buttonCount;
	const unsigned char* buttons = glfwGetJoystickButtons(m_glfwIndex, &buttonCount);

	for (int i = 0; i < buttonCount; i++)
	{
		int buttonCode = i; // 352 is where the controller button codes start in keycodes.h
		// button down
		if (buttons[i] == GLFW_PRESS) {
			buttons_down[buttonCode] = true;
		}
		else
			buttons_down[buttonCode] = false;
		// button press
		if (buttons_down[buttonCode] && !buttons_down_last_frame[buttonCode])
			buttons_pressed[buttonCode] = true;
		else
			buttons_pressed[buttonCode] = false;
		buttons_down_last_frame[buttonCode] = buttons_down[buttonCode];
	}

	// Get axes
	int axesCount;
	const float* axes = glfwGetJoystickAxes(m_glfwIndex, &axesCount);
	float deadZoneLimit = 0.9f;

	if (m_type == Type::XBOX)
	{
		if (axes[4] > deadZoneLimit)
			buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_L] = true;
		else
			buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_L] = false;
		// button press
		if (buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_L] && !buttons_down_last_frame[HELL_XBOX_CONTROLLER_TRIGGER_L])
			buttons_pressed[HELL_XBOX_CONTROLLER_TRIGGER_L] = true;
		else
			buttons_pressed[HELL_XBOX_CONTROLLER_TRIGGER_L] = false;
		buttons_down_last_frame[HELL_XBOX_CONTROLLER_TRIGGER_L] = buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_L];

		if (axes[5] > deadZoneLimit)
			buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_R] = true;
		else
			buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_R] = false;
		// button press
		if (buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_R] && !buttons_down_last_frame[HELL_XBOX_CONTROLLER_TRIGGER_R])
			buttons_pressed[HELL_XBOX_CONTROLLER_TRIGGER_R] = true;
		else
			buttons_pressed[HELL_XBOX_CONTROLLER_TRIGGER_R] = false;
		buttons_down_last_frame[HELL_XBOX_CONTROLLER_TRIGGER_R] = buttons_down[HELL_XBOX_CONTROLLER_TRIGGER_R];

		// Sticks
		left_stick_axis_X = axes[0];
		left_stick_axis_Y = axes[1];
		right_stick_axis_X = axes[2];
		right_stick_axis_Y = axes[3];
	}
		
	if (m_type == Type::PS4)
	{
		if (axes[3] > deadZoneLimit)
			buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_L] = true;
		else
			buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_L] = false;
		// button press
		if (buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_L] && !buttons_down_last_frame[HELL_PS_4_CONTROLLER_TRIGGER_L])
			buttons_pressed[HELL_PS_4_CONTROLLER_TRIGGER_L] = true;
		else
			buttons_pressed[HELL_PS_4_CONTROLLER_TRIGGER_L] = false;
		buttons_down_last_frame[HELL_PS_4_CONTROLLER_TRIGGER_L] = buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_L];

		if (axes[4] > deadZoneLimit)
			buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_R] = true;
		else
			buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_R] = false;
		// button press
		if (buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_R] && !buttons_down_last_frame[HELL_PS_4_CONTROLLER_TRIGGER_R])
			buttons_pressed[HELL_PS_4_CONTROLLER_TRIGGER_R] = true;
		else
			buttons_pressed[HELL_PS_4_CONTROLLER_TRIGGER_R] = false;
		buttons_down_last_frame[HELL_PS_4_CONTROLLER_TRIGGER_R] = buttons_down[HELL_PS_4_CONTROLLER_TRIGGER_R];

		// Sticks
		left_stick_axis_X = axes[0];
		left_stick_axis_Y = axes[1];
		right_stick_axis_X = axes[2];
		right_stick_axis_Y = axes[5];
	}
}

