#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <wincodec.h>

// dxguid.lib
// #include <initguid.h> 
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "PathHelper.h"

#include "D3D12Buffer.h"
#include "D3D12UploadBuffer.h"
#include "D3D12DefaultBuffer.h"
#include "D3D12View.h"
#include "D3D12Sampler.h"
#include "D3D12Resource.h"

#include "Transform.h"

#include "D3D12BufferManager.h"
#include "D3D12DescriptorHeapManager.h"
