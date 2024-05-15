#include <Windows.h>
#include "Window.h"
#include "Renderer.h"
#include "Triangle.h"

using namespace std;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	uint64_t frameIndex = 0;
	MSG msg = { 0 };
	Window window(800, 600); // Create a desktop window
	Renderer renderer(window);
	Triangle triangle(renderer);

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else
		{
			uint32_t frameResourceIndex = (frameIndex++) % 2;

			// Draw
			renderer.BeginFrame(frameResourceIndex);
			triangle.draw(renderer);
			renderer.EndFrame();
		}
	}

	return 0;
}