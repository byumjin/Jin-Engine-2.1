#include "Interface.h"

static void onWindowResized(GLFWwindow* window, int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	Interface* interface = reinterpret_cast<Interface*>(glfwGetWindowUserPointer(window));
	interface->window_Width = width;
	interface->window_Height = height;
}

static void mouseDownCallback(GLFWwindow* window, int button, int action, int mods)
{
	Interface* interface = reinterpret_cast<Interface*>(glfwGetWindowUserPointer(window));

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			leftMouseDown = true;
			glfwGetCursorPos(window, &interface->previousX, &interface->previousY);
		}
		else if (action == GLFW_RELEASE) {
			leftMouseDown = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			rightMouseDown = true;
			glfwGetCursorPos(window, &interface->previousX, &interface->previousY);
		}
		else if (action == GLFW_RELEASE) {
			rightMouseDown = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS) {
			middleMouseDown = true;
			glfwGetCursorPos(window, &interface->previousX, &interface->previousY);
		}
		else if (action == GLFW_RELEASE) {
			middleMouseDown = false;
		}
	}
}

static void mouseMoveCallback(GLFWwindow* window, double xPosition, double yPosition)
{
	Interface* interface = reinterpret_cast<Interface*>(glfwGetWindowUserPointer(window));

	if (leftMouseDown)
	{
		interface->mouseDeltaX += interface->previousX - xPosition;
		interface->mouseDeltaY += interface->previousY - yPosition;

		interface->previousX = xPosition;
		interface->previousY = yPosition;
	}
	else if (rightMouseDown)
	{

	}
}

static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Interface* interface = reinterpret_cast<Interface*>(glfwGetWindowUserPointer(window));

	interface->mouseDeltaZ -= yoffset;
	//interface->previousZ = yoffset;
}

static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_REPEAT || action == GLFW_PRESS)
	{
		Interface* interface = reinterpret_cast<Interface*>(glfwGetWindowUserPointer(window));

		//Alt + Enter
		if (key == GLFW_KEY_ENTER && mods == 4)
		{
			interface->changeWindowMode();
		}
	}
}

void Interface::initWindow()
{
	glfwInit();

	setPrimaryMonitor();
	getResolution();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	if(bfullScreen)
		window = glfwCreateWindow(fullwindow_Width, fullwindow_Height, engineName, primaryMonitor, nullptr);
	else
		window = glfwCreateWindow(window_Width, window_Height, engineName, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, onWindowResized);
	glfwSetMouseButtonCallback(window, mouseDownCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetKeyCallback(window, keyboardCallback);
}

void Interface::getAsynckeyState()
{
	if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
	{
		bFoward = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
	{
		bBackward = true;
	}

	if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
	{
		bLeft = true;
	}

	if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
	{
		bRight = true;
	}

	//Roughness
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET))
	{
		gRoughness -= 0.01f;

		if (gRoughness < 0.01f)
			gRoughness = 0.01f;

	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET))
	{
		gRoughness += 0.01f;

		if (gRoughness > 1.0f)
			gRoughness = 1.0f;
	}

	//Intensity
	if (glfwGetKey(window, GLFW_KEY_COMMA))
	{
		gIntensity -= 0.01f;

		if (gIntensity < 0.00f)
			gIntensity = 0.00f;

	}

	if (glfwGetKey(window, GLFW_KEY_PERIOD))
	{
		gIntensity += 0.01f;

		if (gIntensity > 2.0f)
			gIntensity = 2.0f;
	}
}

void Interface::shutDown()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Interface::setPrimaryMonitor()
{
	primaryMonitor = glfwGetPrimaryMonitor();
}

void Interface::getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(primaryMonitor);

	fullwindow_Width = mode->width;
	fullwindow_Height = mode->height;
}

const char* Interface::getEngineName()
{
	return engineName;
}

GLFWwindow* Interface::getWindow()
{
	return window;
}