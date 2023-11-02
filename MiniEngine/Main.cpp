#include "stdafx.h"
#include "MiniEngine.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    MiniEngine engine(1920, 1080, L"MiniEngine");
    return Win32Application::Run(&engine, hInstance, nCmdShow);
}