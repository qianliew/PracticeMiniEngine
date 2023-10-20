#pragma once

#include "Win32Application.h"

class Window
{
public:
    Window(UINT width, UINT height, std::wstring name);
    virtual ~Window();

    // Accessors.
    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }
    const WCHAR* GetTitle() const { return title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);
    void SetCustomWindowText(LPCWSTR text);

protected:
    UINT width;
    UINT height;
    float aspectRatio;

private:
    // Window title.
    std::wstring title;
};
