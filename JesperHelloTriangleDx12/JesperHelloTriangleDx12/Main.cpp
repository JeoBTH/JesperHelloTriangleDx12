#include <Windows.h>
#include "Window.h"

using namespace std;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	Window window(800, 600); // Create a desktop window


	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}


		// Draw
		// renderer.beginFrame();
		// Render all your stuff!
		// triangle.draw(renderer);
		// renderer.endFrame(); // put the result on the screen, by swapping the buffers


	}

	return 0;
}