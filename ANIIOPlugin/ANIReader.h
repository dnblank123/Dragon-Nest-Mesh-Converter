/*
ANI读取插件for FBX SDK
*/
#include "translate.h"
class ANIReader : public KFbxReader
{
public:
    ANIReader(KFbxSdkManager &pFbxSdkManager, int pID);
    virtual ~ANIReader();
    virtual void GetVersion(int& pMajor, int& pMinor, int& pRevision) const;
    virtual bool FileOpen(char* pFileName);
    virtual bool FileClose();
    virtual bool IsFileOpen();
    virtual bool GetReadOptions(bool pParseFileAsNeeded = true);
    virtual bool Read(KFbxDocument* pDocument);

    //自定义函数声明
    virtual bool InitialPointers(char*, char*&, char*&, AniHeader*&);
    virtual bool AddBoneInfo(KFbxNode*, char*, AniHeader*);
private:
    FILE *mFilePointer;
    KFbxSdkManager *mManager;
};

/*--------------------ANI插件类定义--------------------*/
ANIReader::ANIReader(KFbxSdkManager &pFbxSdkManager, int pID):
KFbxReader(pFbxSdkManager, pID),
mFilePointer(NULL),
mManager(&pFbxSdkManager)
{
}

ANIReader::~ANIReader()
{
    FileClose();
}

void ANIReader::GetVersion(int& pMajor, int& pMinor, int& pRevision) const
{
    pMajor = 1;
    pMinor = 0;
    pRevision = 0;
}

bool ANIReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
        FileClose();
    mFilePointer = fopen(pFileName, "rb");  //二进制方式读取
    if(mFilePointer == NULL)
        return false;
    return true;
}

bool ANIReader::FileClose()
{
    if(mFilePointer!=NULL)
        fclose(mFilePointer);
    return true;

}
bool ANIReader::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

bool ANIReader::GetReadOptions(bool pParseFileAsNeeded)
{
    return true;
}

/*--------------------读取ANI信息--------------------*/
bool ANIReader::Read(KFbxDocument* pDocument)
{
    if (!pDocument){
        GetError().SetLastErrorID(eINVALID_DOCUMENT_HANDLE);
        return false;
    }
    //创建根场景
    KFbxScene*      lScene = KFbxCast<KFbxScene>(pDocument);
    bool            lResult = false;

    if(lScene == NULL)
        return false;

    //初始化根结点
    KFbxNode* lRootNode = lScene->GetRootNode();

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

bool ANIReader::InitialPointers(char* pReader,
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

bool ANIReader::AddBoneInfo(KFbxNode* pRootNode,
                 char* pReader,    //动画数据开始处
                 AniHeader* pHeader)
{
    BoneInfo* pBoneInfo;
    KFbxNode *pParentNode, *pChildNode;
    int *transformationCount, *rotationCount, *scalingCount;
    KFbxXMatrix parentMatrix, childMatrix;
    //为每个骨骼结点添加父子信息
    for(int i = 0; i < pHeader->boneCount; i++){
        pBoneInfo = (BoneInfo*)pReader;
        pParentNode = pRootNode->FindChild(pBoneInfo->parentName);
        pChildNode = pRootNode->FindChild(pBoneInfo->boneName);
        //插入骨骼节点关系信息
        if(pChildNode){ //判断ANI文件正确性
            if(pParentNode == NULL){
                pChildNode->GetSkeleton()->SetSkeletonType(KFbxSkeleton::eROOT);    //根节点时设为root骨骼节点
                childMatrix.SetTRS(pChildNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET),
                                   pChildNode->GetGeometricRotation(KFbxNode::eSOURCE_SET),
                                   pChildNode->GetGeometricScaling(KFbxNode::eSOURCE_SET));
                pChildNode->LclTranslation.Set(childMatrix.GetT());
                pChildNode->LclRotation.Set(childMatrix.GetR());
                pChildNode->LclScaling.Set(childMatrix.GetS());
            }
            else{
                pRootNode->RemoveChild(pChildNode);    //添加节点前先从根场景删除骨骼节点
                pParentNode->AddChild(pChildNode);
                //世界坐标到本地坐标的变换
                parentMatrix.SetTRS(pParentNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET),
                                    pParentNode->GetGeometricRotation(KFbxNode::eSOURCE_SET),
                                    pParentNode->GetGeometricScaling(KFbxNode::eSOURCE_SET));
                childMatrix.SetTRS(pChildNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET),
                                   pChildNode->GetGeometricRotation(KFbxNode::eSOURCE_SET),
                                   pChildNode->GetGeometricScaling(KFbxNode::eSOURCE_SET));
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
