#pragma once
#include "Header.h"
#include "Camera.h"
#include "Renderer/Renderer.h"

	class CoreGL
	{
	public: // functions
		static void InitGL(int windowWidth, int windowHeight);
		static void OnUpdate();
		//static void SwapBuffersAndPollEvents();
		static double GetGLTime();
		static bool IsRunning();
		static void Terminate();
		static void SetVSync(bool enabled);

		static void ToggleFullScreen();

		static void SetFullscreen(bool fullscreen);
		static bool IsFullscreen(void);

		static void CheckGLError_(const char* file, int line);
		static void ClearDefaultFrameBufferToBlack();
		
		//static void SetCamera(Camera* camera);

		static double GetFrameTime();
		static double GetFPS();

		//static void ToggleFullscreen();

	public: // static variables
		static GLFWwindow* s_window;
		static GLFWmonitor* s_monitor; 
		static const GLFWvidmode* s_mode;
		static int s_currentWidth;
		static int s_currentHeight;
		static int s_fullscreenWidth;
		static int s_fullscreenHeight;

	//	static const GLFWvidmode s_desktopMode;

		static std::vector<ScreenResolution> s_resolutions;

	private:
		static int s_windowedWidth;
		static int s_windowedHeight;
		static int windowedPosX;
		static int windowedPosY;
		//static Camera* p_camera;

		static double lastTime;
		static int nbFrames;
		static double s_frameTime;
		static double s_fps;

		
	};
