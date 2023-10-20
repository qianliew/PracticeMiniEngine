#include "stdafx.h"
#include "Window.h"

Window::Window(UINT width, UINT height, std::wstring name) :
    width(width),
    height(height),
    title(name)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));

    aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

Window::~Window()
{

}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void Window::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            title = title + L" (WARP)";
        }
    }
}

// Helper function for setting the window's title text.
void Window::SetCustomWindowText(LPCWSTR text)
{
    std::wstring windowText = title + L": " + text;
    SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}
