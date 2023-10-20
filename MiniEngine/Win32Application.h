#pragma once
#include "MiniEngine.h"

class MiniEngine;

class Win32Application
{
public:
    static int Run(MiniEngine* pEngine, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
};
