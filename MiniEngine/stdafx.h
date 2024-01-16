#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <wincodec.h>

// dxguid.lib
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
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "SharedPrimitives.h"
#include "SharedConstants.h"
#include "SharedTypes.h"

#include "Macros.h"
#include "PathHelper.h"

#include "D3D12Buffer.h"
#include "D3D12UploadBuffer.h"
#include "D3D12DefaultBuffer.h"
#include "D3D12ReadbackBuffer.h"
#include "D3D12View.h"
#include "D3D12Sampler.h"
#include "D3D12Resource.h"
#include "D3D12ConstantBuffer.h"

#include "D3D12Device.h"
#include "D3D12RootSignature.h"
#include "D3D12CommandList.h"
