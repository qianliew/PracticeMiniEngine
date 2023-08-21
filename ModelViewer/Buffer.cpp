#include "stdafx.h"
#include "Buffer.h"

ComPtr<ID3D12Resource> Buffer::GetBuffer()
{
    return m_buffer;
}

void* Buffer::GetStartLocation()
{
    return m_startLocation;
}

UINT Buffer::GetDataSize()
{
    return m_dataSize;
}