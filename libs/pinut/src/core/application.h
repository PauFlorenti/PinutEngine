#pragma once

#include "device.h"
#include "swapchain.h"

struct GLFWwindow;

namespace Pinut
{
	class Application
	{
	public:
		Application(const char *inName, int32_t inWidth = 1920, int32_t inHeight = 1080);
		virtual ~Application(){};

		// Application interface
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize() = 0;
		virtual void OnUpdateDisplay() = 0;

		void DeviceInit(GLFWwindow *window);
		void DeviceShutdown();

		void WaitIdle();

		virtual void OnUpdate();
		void UpdateDisplay();
		static void OnWindowMoved(GLFWwindow *window, int x, int y);
		static void OnWindowResized(GLFWwindow *window, int inWidth, int inHeight);

		const std::string GetName() const { return name; }
		const int32_t GetWidth() const { return width; }
		const int32_t GetHeight() const { return height; }

	protected:
		std::string name;
		int32_t width;	// app windows dimensions
		int32_t height; // app windows dimension

		double deltaTime;
		double lastFrameTime;

		GLFWwindow *windowHandle{nullptr};

		bool bCPUValidationEnabled;
		bool bGPUValidationEnabled;

		Device device;
		Swapchain swapchain;
	};
}

int32_t Run(Pinut::Application *application);
