#include "stdafx.h"
#include "FBXImporter.h"

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(m_fbxManager->GetIOSettings()))
#endif

FBXImporter::FBXImporter()
{

}

FBXImporter::~FBXImporter()
{
    if (m_fbxManager)
    {
        m_fbxManager->Destroy();
    }
}

void FBXImporter::InitializeSdkObjects()
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    m_fbxManager = FbxManager::Create();
    if (!m_fbxManager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
    else FBXSDK_printf("Autodesk FBX SDK version %s\n", m_fbxManager->GetVersion());

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(m_fbxManager, IOSROOT);
    m_fbxManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    m_fbxManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    m_fbxScene = FbxScene::Create(m_fbxManager, "My Scene");
    if (!m_fbxScene)
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

bool FBXImporter::ImportFBX(char* path)
{
    FbxString lfilePath(path);
	if (lfilePath.IsEmpty())
	{
        return false;
	}

    FBXSDK_printf("\n\nFile: %s\n\n", lfilePath.Buffer());
    const char* pFilename = lfilePath.Buffer();

    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor, lSDKMinor, lSDKRevision;
    //int lFileFormat = -1;
    int lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(m_fbxManager, "");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, m_fbxManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if (!lImportStatus)
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for (int i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(m_fbxScene);
    if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
    {
        FBXSDK_printf("Please enter password: ");

        lPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(m_fbxScene);

        if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    if (!lStatus || (lImporter->GetStatus() != FbxStatus::eSuccess))
    {
        FBXSDK_printf("********************************************************************************\n");
        if (lStatus)
        {
            FBXSDK_printf("WARNING:\n");
            FBXSDK_printf("   The importer was able to read the file but with errors.\n");
            FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
        }
        else
        {
            FBXSDK_printf("Importer failed to load the file!\n\n");
        }

        if (lImporter->GetStatus() != FbxStatus::eSuccess)
            FBXSDK_printf("   Last error message: %s\n", lImporter->GetStatus().GetErrorString());

        FbxArray<FbxString*> history;
        lImporter->GetStatus().GetErrorStringHistory(history);
        if (history.GetCount() > 1)
        {
            FBXSDK_printf("   Error history stack:\n");
            for (int i = 0; i < history.GetCount(); i++)
            {
                FBXSDK_printf("      %s\n", history[i]->Buffer());
            }
        }
        FbxArrayDelete<FbxString*>(history);
        FBXSDK_printf("********************************************************************************\n");
    }

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

void FBXImporter::LoadFBX(const std::shared_ptr<D3D12Mesh>& mesh)
{
    FbxGeometryConverter converter(m_fbxManager);
    converter.Triangulate(m_fbxScene, true);
    LoadContent(m_fbxScene, mesh);
}

void FBXImporter::LoadContent(FbxScene* pScene, const std::shared_ptr<D3D12Mesh>& mesh)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if (lNode)
    {
        for (i = 0; i < lNode->GetChildCount(); i++)
        {
            LoadContent(lNode->GetChild(i), mesh);
        }
    }
}

void FBXImporter::LoadContent(FbxNode* pNode, const std::shared_ptr<D3D12Mesh>& mesh)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if (pNode->GetNodeAttribute() == NULL)
    {
        FBXSDK_printf("NULL Node Attribute\n\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        default:
            break;

        case FbxNodeAttribute::eMesh:
            LoadMesh(pNode, mesh);
            break;
        }
    }

    for (i = 0; i < pNode->GetChildCount(); i++)
    {
        LoadContent(pNode->GetChild(i), mesh);
    }
}

void FBXImporter::LoadMesh(FbxNode* pNode, const std::shared_ptr<D3D12Mesh>& mesh)
{
    FbxMesh* lMesh = (FbxMesh*)pNode->GetNodeAttribute();
    UINT polygonSize = lMesh->GetPolygonCount();

    UINT indicesSize = polygonSize * 3 * sizeof(UINT16);
    UINT16* pIndex = (UINT16*)malloc(indicesSize);
    UINT16* iIndex = pIndex;

    int lControlPointsCount = lMesh->GetControlPointsCount();
    UINT verticesSize = polygonSize * 3 * sizeof(Vertex);
    fbxsdk::FbxVector4* lControlPoints = lMesh->GetControlPoints();

    Vertex* pVertex = (Vertex*)malloc(verticesSize);
    Vertex* iVertex = pVertex;

    if (pIndex && pVertex)
    {
        for (int i = 0; i < polygonSize; i++)
        {
            for (int j = 0; j < lMesh->GetPolygonSize(i); j++)
            {
                *(iIndex++) = i * 3 + j;
                int cpIndex = lMesh->GetPolygonVertex(i, j);
                iVertex->position = XMFLOAT3
                {
                    static_cast<float>(lControlPoints[cpIndex].mData[0]),
                    static_cast<float>(lControlPoints[cpIndex].mData[1]),
                    static_cast<float>(lControlPoints[cpIndex].mData[2]),
                };

                FbxGeometryElementUV* leUV = lMesh->GetElementUV(0);

                switch (leUV->GetMappingMode())
                {
                default:
                    break;
                case FbxGeometryElement::eByControlPoint:
                    switch (leUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        iVertex->texCoord = XMFLOAT2
                        {
                            static_cast<float>(leUV->GetDirectArray().GetAt(cpIndex).mData[0]),
                            static_cast<float>(leUV->GetDirectArray().GetAt(cpIndex).mData[1]),
                        };
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = leUV->GetIndexArray().GetAt(cpIndex);
                        iVertex->texCoord = XMFLOAT2
                        {
                            static_cast<float>(leUV->GetDirectArray().GetAt(id).mData[0]),
                            static_cast<float>(leUV->GetDirectArray().GetAt(id).mData[1]),
                        };
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                    break;

                case FbxGeometryElement::eByPolygonVertex:
                {
                    int lTextureUVIndex = lMesh->GetTextureUVIndex(i, j);
                    switch (leUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        iVertex->texCoord = XMFLOAT2
                        {
                            static_cast<float>(leUV->GetDirectArray().GetAt(lTextureUVIndex).mData[0]),
                            static_cast<float>(leUV->GetDirectArray().GetAt(lTextureUVIndex).mData[1]),
                        };
                    }
                    break;
                    default:
                        break; // other reference modes not shown here!
                    }
                }
                break;

                case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                    break;
                }

                (iVertex++)->color = XMFLOAT4
                {
                    1, 1, 1, 1
                };
            }
        }

        mesh->SetIndices(pIndex, indicesSize);
        mesh->SetVertices(pVertex, verticesSize);

        delete pIndex;
        delete pVertex;
    }
}
