#pragma once

#include "Common.h"
#include "Time.h"

static bool leftMouseDown = false;
static bool rightMouseDown = false;
static bool middleMouseDown = false;

class Interface
{
public:

	Interface():window(NULL), primaryMonitor(NULL), window_Width(800), window_Height(600), bfullScreen(false), engineName("Jin Engine")
	{
		mouseDeltaX = 0.0;
		mouseDeltaY = 0.0;
		mouseDeltaZ = 0.0;

		previousX = 0.0;
		previousY = 0.0;
		previousZ = 0.0;

		fps = 0;
		fpstracker = 0;

		bFoward = false;
		bBackward = false;
		bLeft = false;
		bRight = false;

		windowResetFlag = false;

		//SSR
		gRoughness = 0.0f;
		gIntensity = 0.3f;

		bRotate = false;
		bUseNormalMap = false;
		bUseHolePatching = true;
		SSRVisibility = 0;
		bUseBruteForce = false;
		bUseInterpolation = false;
		bMoveForward = false;

	}

	void shutDown();

	void setPrimaryMonitor();
	void getResolution();

	void initWindow();
	
	void setWindowTitle(std::string title)
	{
		glfwSetWindowTitle(window, title.c_str());
	}

	void changeWindowMode()
	{
		shutDown();
		bfullScreen = !bfullScreen;
		windowResetFlag = true;		
	}

	void getAsynckeyState();
		
	int fps;
	int fpstracker;

	const char* getEngineName();
	GLFWwindow* getWindow();
	
	double mouseDeltaX;
	double mouseDeltaY;
	double mouseDeltaZ;

	double previousX;
	double previousY;
	double previousZ;

	bool bFoward;
	bool bBackward;
	bool bLeft;
	bool bRight;

	int window_Width;
	int window_Height;

	bool windowResetFlag;

	bool bRotate;
	bool bMoveForward;

	//SSR
	float gRoughness;
	float gIntensity;
	bool bUseNormalMap;
	bool bUseHolePatching;
	bool bUseBruteForce;
	bool bUseInterpolation;

	int SSRVisibility;

private:
	GLFWwindow* window;
	GLFWmonitor* primaryMonitor;
	
	bool bfullScreen;

	

	int fullwindow_Width;
	int fullwindow_Height;

	const char* engineName;
};




