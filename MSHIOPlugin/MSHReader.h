/*
MSH读取插件for FBX SDK
*/
#pragma once

class MSHReader : public FbxReader
{
public:
    MSHReader(FbxManager& pFbxSdkManager, int pID, FbxStatus& pStatus);
    virtual ~MSHReader();
    virtual void GetVersion(int& pMajor, int& pMinor, int& pRevision) const;
    virtual bool FileOpen(char* pFileName);
    virtual bool FileClose();
    virtual bool IsFileOpen();
    virtual bool GetReadOptions(bool pParseFileAsNeeded = true);
    virtual bool Read(FbxDocument* pDocument);

    //自定义函数声明
    virtual char* InitialPointer(char*, char*&, MshHeader*&);
    virtual char* InitialMeshPointer(char*, MeshInfo*&, MeshDataPointer*);
    virtual FbxNode* CreateMesh(FbxScene*, MeshInfo*, MeshDataPointer*);
    virtual void CreateSkeleton(FbxScene*, MshHeader*, BoneData*);
    virtual FbxCluster** LinkSkeleton(FbxNode*, FbxNode*, MeshDataPointer*);
    virtual void AddWeight(FbxCluster**, MeshInfo*, MeshDataPointer*);
    virtual void StoreBindPose(FbxScene*, FbxNode*, FbxCluster**, MeshDataPointer*);
private:
    FILE *mFilePointer;
    FbxManager *mManager;
};

/*--------------------MSH插件类定义--------------------*/
inline MSHReader::MSHReader(FbxManager& pFbxSdkManager, int pID, FbxStatus& pStatus):
FbxReader(pFbxSdkManager, pID, pStatus),
mFilePointer(NULL),
mManager(&pFbxSdkManager)
{
}

inline MSHReader::~MSHReader()
{
    FileClose();
}

inline void MSHReader::GetVersion(int& pMajor, int& pMinor, int& pRevision) const
{
    pMajor = 1;
    pMinor = 0;
    pRevision = 0;
}

inline bool MSHReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
        FileClose();
    mFilePointer = fopen(pFileName, "rb");  //二进制方式读取
    if(mFilePointer == NULL)
        return false;
    return true;
}

inline bool MSHReader::FileClose()
{
    if(mFilePointer!=NULL)
        fclose(mFilePointer);
    return true;

}
inline bool MSHReader::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

inline bool MSHReader::GetReadOptions(bool pParseFileAsNeeded)
{
    return true;
}

/*--------------------读取MSH信息--------------------*/
inline bool MSHReader::Read(FbxDocument* pDocument)
{
    FbxStatus status;
    if (!pDocument) {
        status.SetCode(FbxStatus::eFailure, "Invalid document handle");
        return false;
    }
    //创建根场景
    FbxScene*      lScene = FbxCast<FbxScene>(pDocument);
    bool            lResult = false;

    if(lScene == NULL)
        return false;

    //初始化根结点
    FbxNode* lRootNode = lScene->GetRootNode();
    FbxNodeAttribute * lRootNodeAttribute = FbxNull::Create(lScene,"Scene Root");
    lRootNode->SetNodeAttribute(lRootNodeAttribute);

    if(mFilePointer == NULL)
        return true;

    //获取文件大小
    unsigned lSize;  //文件大小
    char* lBuffer = NULL;
    fseek(mFilePointer, 0, SEEK_END);
    lSize = ftell(mFilePointer);
    rewind(mFilePointer);
    //读入文件到lBuffer
    lBuffer = (char*) malloc (sizeof(char)*lSize + 1);
    size_t lRead = fread(lBuffer, 1, lSize, mFilePointer);
    lBuffer[lRead] = NULL;

    //创建工作指针
    char *pBone, *pReader;
    MshHeader* pHeader;
    MeshInfo* pMeshInfo;
    MeshDataPointer meshData;
    //初始化各指针
    pReader = InitialPointer(lBuffer, pBone, pHeader);

    //创建骨骼
    if(pHeader->boneCount > 0){
        BoneData* pBoneData = (BoneData*)pBone;
        CreateSkeleton(lScene, pHeader, pBoneData);

        //创建meshCount个mesh模型并与骨骼绑定
        for(int i = 0; i < pHeader->meshCount; i++){
             //初始化Mesh的指针
            pReader = InitialMeshPointer(pReader, pMeshInfo, &meshData);

            //读取并创建Mesh
            FbxNode* pMesh = CreateMesh(lScene, pMeshInfo, &meshData);
            lRootNode->AddChild(pMesh);

            //连接骨骼到Mesh
            FbxCluster** pClusterIndex = LinkSkeleton(lRootNode, pMesh, &meshData);
            //添加骨骼权重
            AddWeight(pClusterIndex, pMeshInfo, &meshData);
            //添加BindPose
            StoreBindPose(lScene, pMesh, pClusterIndex, &meshData);

        }
    }
    //释放缓冲
    free(lBuffer);

    lResult = true;
    return lResult;
}

/*--------------------自定义函数定义--------------------*/
inline char* MSHReader::InitialPointer(char* pReader,
                                char* &pBone,
                                MshHeader* &pHeader)
{
    //MSH头信息开始处
    pHeader = (MshHeader*)pReader;
    //骨骼数据开始处
    pReader += 0x400;
    pBone = pReader;
    //Mesh信息开始处
    pReader += pHeader->boneCount * 0x140;

    //返回工作指针位置
    return pReader;
}

inline char* MSHReader::InitialMeshPointer(char* pReader,
                                    MeshInfo* &pMeshInfo,
                                    MeshDataPointer* pMeshData)
{
    //Mesh信息开始处
    pMeshInfo = (MeshInfo*)pReader;
    //Mesh数据开始处
    pReader += 0x400;
    pMeshData->pFaceIndex = pReader;
    pReader += pMeshInfo->faceIndexCount * 0x2;
    pMeshData->pVertexData = pReader;
    pReader += pMeshInfo->vertexCount * 0xC;
    pMeshData->pNormalData = pReader;
    pReader += pMeshInfo->vertexCount * 0xC;
    pMeshData->pUVData = pReader;
    pReader += pMeshInfo->vertexCount * 0x8;
    pMeshData->pBoneIndex = pReader;
    pReader += pMeshInfo->vertexCount * 0x8;
    pMeshData->pBoneWeight = pReader;
    pReader += pMeshInfo->vertexCount * 0x10;
    pMeshData->pBoneCount = (int*)pReader;
    pReader += 0x4;
    pMeshData->pBoneName = pReader;

    //返回工作指针位置
    pReader += *pMeshData->pBoneCount * 0x100;  //第n个Mesh数据尾部
    return pReader;
}

inline FbxNode* MSHReader::CreateMesh(FbxScene* pScene,
                                MeshInfo* pMeshInfo,
                                MeshDataPointer* pMeshData)
{
    //创建mesh结点
    FbxMesh* lMesh = FbxMesh::Create(pScene, pMeshInfo->meshName);
    //把mesh结点加入到场景
    FbxNode* lNode = FbxNode::Create(pScene, pMeshInfo->meshName);
    lNode->SetNodeAttribute(lMesh);
    lNode->SetShadingMode(FbxNode::eTextureShading);
    //创建Layer Container
    lMesh->CreateLayer();
    FbxLayer* layer = lMesh->GetLayer(0);
    //连接法线层
    FBX_ASSERT(layer != NULL);
    FbxLayerElementNormal* normLayer;
    normLayer = FbxLayerElementNormal::Create((FbxLayerContainer*)lMesh, "");
    normLayer->SetMappingMode(FbxLayerElement::eByPolygonVertex);
    normLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    layer->SetNormals(normLayer);
    //连接UV层
    FbxLayerElementUV* uvLayer;
    uvLayer = FbxLayerElementUV::Create((FbxLayerContainer*)lMesh, "map1");
    uvLayer->SetMappingMode(FbxLayerElement::eByPolygonVertex);
    uvLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    layer->SetUVs(uvLayer);

    //读取顶点信息
    Vec3F* tVertex = (Vec3F*)pMeshData->pVertexData;   //顶点指针,xyz三个float
    lMesh->InitControlPoints(pMeshInfo->vertexCount);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();
    for(int i = 0; i < pMeshInfo->vertexCount; i++){
        lControlPoints[i] = FbxVector4(tVertex->x, tVertex->y, tVertex->z);
        tVertex++;
    }

    //读取法线信息
    Vec3F* tNormal = (Vec3F*)pMeshData->pNormalData;
    for(int i = 0; i < pMeshInfo->vertexCount; i++){
        normLayer->GetDirectArray().Add(FbxVector4(tNormal->x, tNormal->y, tNormal->z));
        tNormal++;
    }

    //读取UV信息
    Vec2F* pUV = (Vec2F*)pMeshData->pUVData;
    for(int i = 0; i < pMeshInfo->vertexCount; i++){
        uvLayer->GetDirectArray().Add(FbxVector2(pUV->x, 1 - pUV->y)); //垂直翻转
        pUV++;
    }

    //根据顶点索引信息建立Mesh
    short* pFace = (short*)pMeshData->pFaceIndex;
    //GL_TRIANGLES 模式
    if(pMeshInfo->renderMode == 0x100)
        for(; pFace + 2 <= (short*)pMeshData->pVertexData;){
            lMesh->BeginPolygon();
            for(int j = 0; j < 3; j++){
                lMesh->AddPolygon(*pFace);
                uvLayer->GetIndexArray().Add(*pFace);
                normLayer->GetIndexArray().Add(*pFace);
                pFace++;
            }
            lMesh->EndPolygon();
        }
    //GL_TRIANGLE_STRIP 模式
    else{
        for(unsigned i = 2; i < pMeshInfo->faceIndexCount; i++){
            lMesh->BeginPolygon();
            if(i & 1){
                lMesh->AddPolygon(pFace[0]);
                lMesh->AddPolygon(pFace[2]);
                lMesh->AddPolygon(pFace[1]);
                uvLayer->GetIndexArray().Add(pFace[0]);
                uvLayer->GetIndexArray().Add(pFace[2]);
                uvLayer->GetIndexArray().Add(pFace[1]);
                normLayer->GetIndexArray().Add(pFace[0]);
                normLayer->GetIndexArray().Add(pFace[2]);
                normLayer->GetIndexArray().Add(pFace[1]);
            }
            else{
                lMesh->AddPolygon(pFace[0]);
                lMesh->AddPolygon(pFace[1]);
                lMesh->AddPolygon(pFace[2]);
                uvLayer->GetIndexArray().Add(pFace[0]);
                uvLayer->GetIndexArray().Add(pFace[1]);
                uvLayer->GetIndexArray().Add(pFace[2]);
                normLayer->GetIndexArray().Add(pFace[0]);
                normLayer->GetIndexArray().Add(pFace[1]);
                normLayer->GetIndexArray().Add(pFace[2]);
            }
            lMesh->EndPolygon();
            pFace++;
        }
    }

    lMesh->RemoveBadPolygons();

    return lNode;
}

//创建骨骼
inline void MSHReader::CreateSkeleton(FbxScene* pScene,
                               MshHeader* pHeader,
                               BoneData* pBoneData)
{
    for(int i = 0; i < pHeader->boneCount; i++){
        FbxSkeleton* lSkeleton = FbxSkeleton::Create(pScene, "");
        lSkeleton->SetSkeletonType(FbxSkeleton::eLimbNode);
        lSkeleton->Size.Set(50.0);
        FbxNode* lNode = FbxNode::Create(pScene, pBoneData->boneName);
        lNode->SetNodeAttribute(lSkeleton);

        //添加变换矩阵到骨骼
        //需要注意原数据是骨骼世界变换矩阵的逆矩阵
        double tMatrix[4][4];
        for(int i = 0; i < 4; i++){ //单精度到双精度的转换
            tMatrix[i][0] = pBoneData->transformMatrix[i].x;
            tMatrix[i][1] = pBoneData->transformMatrix[i].y;
            tMatrix[i][2] = pBoneData->transformMatrix[i].z;
            tMatrix[i][3] = pBoneData->transformMatrix[i].w;
        }
        FbxAMatrix pMatrix = *(FbxAMatrix*)&tMatrix;
        pMatrix = pMatrix.Inverse();

        //定位骨骼位置
        lNode->LclTranslation.Set(pMatrix.GetT());
        lNode->LclRotation.Set(pMatrix.GetR());
        lNode->LclScaling.Set(pMatrix.GetS());
        //设定骨骼变换矩阵信息
        lNode->SetGeometricTranslation(FbxNode::eSourcePivot, pMatrix.GetT());
        lNode->SetGeometricRotation(FbxNode::eSourcePivot, pMatrix.GetR());
        lNode->SetGeometricScaling(FbxNode::eSourcePivot, pMatrix.GetS());

        //添加到场景根节点
        pScene->GetRootNode()->AddChild(lNode);

        //下一个骨骼数据
        pBoneData++;
    }
}

//连接相应骨骼到Mesh,返回值是一个存放约束器指针的数组
inline FbxCluster** MSHReader::LinkSkeleton(FbxNode* pScene,
                                      FbxNode* pMesh,
                                      MeshDataPointer* pMeshData)
{
    int meshBoneCount = *pMeshData->pBoneCount;
    char* pBoneName = pMeshData->pBoneName;
    FbxGeometry* lMeshAttribute = (FbxGeometry*)pMesh->GetNodeAttribute();
    FbxSkin* lSkin = FbxSkin::Create(pScene, "");
    //创建骨骼名称对应第n索引,pCluster是指向约束器指针的指针
    FbxCluster** pCluster = (FbxCluster**)malloc(sizeof(FbxCluster*) * meshBoneCount);

    //连接骨骼节点与约束器,追加蒙皮
    for(int i = 0; i < meshBoneCount; i++){
        pCluster[i] = FbxCluster::Create(pScene,"");
        pCluster[i]->SetLink(pScene->FindChild(pBoneName));
        pCluster[i]->SetLinkMode(FbxCluster::eTotalOne);

        //为约束器设定世界变换矩阵
        FbxNode* pChildNode = pScene->FindChild(pBoneName);
        FbxAMatrix childMatrix(pChildNode->GetGeometricTranslation(FbxNode::eSourcePivot),
                                pChildNode->GetGeometricRotation(FbxNode::eSourcePivot),
                                pChildNode->GetGeometricScaling(FbxNode::eSourcePivot));
        pCluster[i]->SetTransformLinkMatrix(childMatrix);

        //添加约束器到蒙皮
        lSkin->AddCluster(pCluster[i]);
        pBoneName += 0x100;
    }
    //添加蒙皮到Mesh
    lMeshAttribute->AddDeformer(lSkin);
    return pCluster;
}

//添加权重
inline void MSHReader::AddWeight(FbxCluster** pClusterIndex,
                          MeshInfo* pMeshInfo,
                          MeshDataPointer* pMeshData)
{
    float* boneWeight = (float*)pMeshData->pBoneWeight;
    short* pBoneIndex = (short*)pMeshData->pBoneIndex;
    for(int i = 0; i < pMeshInfo->vertexCount; i++)
        for(int j = 0; j < 4; j++){  //每个顶点最多4个骨骼权重
            if(*(boneWeight + i * 4 + j) != 0)  //第i个顶点第j个权重索引
                pClusterIndex[*pBoneIndex]->AddControlPointIndex(i, *(boneWeight + i * 4 + j));
            pBoneIndex++;
        }
}

//保存当前Pose
inline void MSHReader::StoreBindPose(FbxScene* pScene,
                              FbxNode* pMesh,
                              FbxCluster** pClusterIndex,
                              MeshDataPointer* pMeshData)
{
    FbxPose* lPose = FbxPose::Create(pScene,pMesh->GetName());
    lPose->SetIsBindPose(true);

    FbxMatrix lBindMeshMatrix = pScene->GetAnimationEvaluator()->GetNodeGlobalTransform(pMesh);
    lPose->Add(pMesh, lBindMeshMatrix);
    for(int i=0; i < *pMeshData->pBoneCount; i++){
        FbxNode* lFbxNode = pClusterIndex[i]->GetLink();
        FbxMatrix lBindMatrix = pScene->GetAnimationEvaluator()->GetNodeGlobalTransform(lFbxNode);
        lBindMatrix = lBindMatrix.Inverse();
        lPose->Add(lFbxNode, lBindMatrix);
    }
    pScene->AddPose(lPose);
}

