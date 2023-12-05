/*
ANI读取插件for FBX SDK
*/
#pragma once
#include "translate.h"

class ANIReader : public FbxReader
{
public:
    ANIReader(FbxManager& pFbxSdkManager, int pID, FbxStatus& pStatus);
    virtual ~ANIReader();
    virtual void GetVersion(int& pMajor, int& pMinor, int& pRevision) const;
    virtual bool FileOpen(char* pFileName);
    virtual bool FileClose();
    virtual bool IsFileOpen();
    virtual bool GetReadOptions(bool pParseFileAsNeeded = true);
    virtual bool Read(FbxDocument* pDocument);

    //自定义函数声明
    virtual bool InitialPointers(char*, char*&, char*&, AniHeader*&);
    virtual bool AddBoneInfo(FbxNode*, char*, AniHeader*);
private:
    FILE *mFilePointer;
    FbxManager *mManager;
};

/*--------------------ANI插件类定义--------------------*/
inline ANIReader::ANIReader(FbxManager& pFbxSdkManager, int pID, FbxStatus& pStatus) :
FbxReader(pFbxSdkManager, pID, pStatus),
mFilePointer(NULL),
mManager(&pFbxSdkManager)
{

}

inline ANIReader::~ANIReader()
{
    FileClose();
}

inline void ANIReader::GetVersion(int& pMajor, int& pMinor, int& pRevision) const
{
    pMajor = 1;
    pMinor = 0;
    pRevision = 0;
}

inline bool ANIReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
        FileClose();
    mFilePointer = fopen(pFileName, "rb");  //二进制方式读取
    if(mFilePointer == NULL)
        return false;
    return true;
}

inline bool ANIReader::FileClose()
{
    if(mFilePointer!=NULL)
        fclose(mFilePointer);
    return true;

}
inline bool ANIReader::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

inline bool ANIReader::GetReadOptions(bool pParseFileAsNeeded)
{
    return true;
}

/*--------------------读取ANI信息--------------------*/
inline bool ANIReader::Read(FbxDocument* pDocument)
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

    if(mFilePointer == NULL)
        return true;

    //获取文件大小
    int lSize;  //文件大小
    char* lBuffer = NULL;
    fseek(mFilePointer, 0, SEEK_END);
    lSize = ftell(mFilePointer);
    rewind(mFilePointer);
    //读入文件到lBuffer
    lBuffer = (char*) malloc (sizeof(char)*lSize + 1);
    size_t lRead = fread(lBuffer, 1, lSize, mFilePointer);
    lBuffer[lRead] = NULL;

    //创建工作指针
    char *pAnimateName, *pAnimateData;
    AniHeader* pHeader;

    //初始化各指针
    lResult = InitialPointers(lBuffer, pAnimateName, pAnimateData, pHeader);

    //添加骨骼父子信息
    lResult = AddBoneInfo(lRootNode, pAnimateData, pHeader);

    //释放缓冲
    free(lBuffer);

    return lResult;
}

/*--------------------自定义函数定义--------------------*/

inline bool ANIReader::InitialPointers(char* pReader,
                     char* &pAnimateName,
                     char* &pAnimateData,
                     AniHeader* &pHeader)
{
    //ANI头信息开始处
    pHeader = (AniHeader*)pReader;
    //动画名称开始处
    pReader += 0x400;
    pAnimateName = pReader;
    //动画帧数列表开始处
    pReader += pHeader->animateCount * 0x100;
    //动画数据开始处
    pReader += pHeader->animateCount * 0x4;
    pAnimateData = pReader;

    return true;
}

inline bool ANIReader::AddBoneInfo(FbxNode* pRootNode,
                 char* pReader,    //动画数据开始处
                 AniHeader* pHeader)
{
    BoneInfo* pBoneInfo;
    FbxNode *pParentNode, *pChildNode;
    int *transformationCount, *rotationCount, *scalingCount;
    FbxAMatrix parentMatrix, childMatrix;
    //为每个骨骼结点添加父子信息
    for(int i = 0; i < pHeader->boneCount; i++){
        pBoneInfo = (BoneInfo*)pReader;
        pParentNode = pRootNode->FindChild(pBoneInfo->parentName);
        pChildNode = pRootNode->FindChild(pBoneInfo->boneName);
        //插入骨骼节点关系信息
        if(pChildNode){ //判断ANI文件正确性
            if(pParentNode == NULL){
                pChildNode->GetSkeleton()->SetSkeletonType(FbxSkeleton::eRoot);    //根节点时设为root骨骼节点
                childMatrix.SetTRS(pChildNode->GetGeometricTranslation(FbxNode::eSourcePivot),
                                   pChildNode->GetGeometricRotation(FbxNode::eSourcePivot),
                                   pChildNode->GetGeometricScaling(FbxNode::eSourcePivot));
                pChildNode->LclTranslation.Set(childMatrix.GetT());
                pChildNode->LclRotation.Set(childMatrix.GetR());
                pChildNode->LclScaling.Set(childMatrix.GetS());
            }
            else{
                pRootNode->RemoveChild(pChildNode);    //添加节点前先从根场景删除骨骼节点
                pParentNode->AddChild(pChildNode);
                //世界坐标到本地坐标的变换
                parentMatrix.SetTRS(pParentNode->GetGeometricTranslation(FbxNode::eSourcePivot),
                                    pParentNode->GetGeometricRotation(FbxNode::eSourcePivot),
                                    pParentNode->GetGeometricScaling(FbxNode::eSourcePivot));
                childMatrix.SetTRS(pChildNode->GetGeometricTranslation(FbxNode::eSourcePivot),
                                   pChildNode->GetGeometricRotation(FbxNode::eSourcePivot),
                                   pChildNode->GetGeometricScaling(FbxNode::eSourcePivot));
                childMatrix = parentMatrix.Inverse() * childMatrix;
                pChildNode->LclTranslation.Set(childMatrix.GetT());
                pChildNode->LclRotation.Set(childMatrix.GetR());
                pChildNode->LclScaling.Set(childMatrix.GetS());
            }
        }
        else{
            /*错误提示*/
            wchar_t errorMSG[128], tName[64];
            wcscpy(errorMSG, LANG_MSG_BONE_NOT_FOUND);
            MultiByteToWideChar(CP_UTF8, NULL, pBoneInfo->boneName, -1, tName, 64);
            wcscat(errorMSG, tName);
            wcscat(errorMSG, LANG_MSG_QUESTION);
            if(MessageBox(0, errorMSG, LANG_WARN, MB_ICONWARNING | MB_OKCANCEL) == IDCANCEL)
                return false;
        }

        //跳过本骨骼动画信息
        pReader += 0x400;    //sizeof(BoneInfo)
        for(int j = 0; j < pHeader->animateCount; j++){
            //指针后移,平移变换数据部分
            pReader += 0x28;    //sizeof(AnimateBaseData)
            transformationCount = (int*)pReader;
            pReader += sizeof(int);
            //指针后移,旋转变换数据部分
            pReader += *transformationCount * 0xE; //sizeof(TransformationData)
            rotationCount = (int*)pReader;
            pReader += sizeof(int);
            //指针后移,旋转变换数据部分
            pReader += *rotationCount * 0xA;   //sizeof(RotationData)
            scalingCount = (int*)pReader;
            pReader += sizeof(int);
            pReader += *scalingCount * 0xE;    //sizeof(ScalingData)
        }
    }

    return true;
}
