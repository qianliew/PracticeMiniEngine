#pragma once
#include <fbxsdk.h>

class FBXImporter
{
public:
    FBXImporter();
    ~FBXImporter();
    void InitializeSdkObjects();
    bool ImportFBX();

private:
    // FBX SDK objects
    FbxManager* m_fbxManager = nullptr;
    FbxScene* m_fbxScene = nullptr;
};