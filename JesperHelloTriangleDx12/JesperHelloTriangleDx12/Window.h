#pragma once
#include <Windows.h>

class Window
{
private:
	int width = 0;
	int height = 0;
	HWND m_handle;

public:
	Window(int width = 800, int height = 600);
	~Window();

	HWND getHandle();

};
