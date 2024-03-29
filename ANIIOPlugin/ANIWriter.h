//This class is a custom writer.
//The writer provide you the ability to write out node hierarchy to a custom file format.
#pragma once

class ANIWriter : public FbxWriter
{
public:

    ANIWriter(FbxManager& pFbxManager, int pID, FbxStatus& pStatus);

    //VERY important to put the file close in the destructor
    virtual ~ANIWriter();

    virtual bool FileCreate(char* pFileName) ;
    virtual bool FileClose() ;
    virtual bool IsFileOpen();
    virtual void GetWriteOptions() ;
    virtual bool Write(FbxDocument* pDocument);
    virtual bool PreprocessScene(FbxScene &pScene);
    virtual bool PostprocessScene(FbxScene &pScene);

private:
    FILE *mFilePointer;
    FbxManager *mManager;
};

inline ANIWriter::ANIWriter(FbxManager& pFbxManager, int pID, FbxStatus& pStatus):
FbxWriter(pFbxManager, pID, pStatus),
mFilePointer(NULL),
mManager(&pFbxManager)
{

}

inline ANIWriter::~ANIWriter()
{
    FileClose();
}

// Create a file stream with pFileName
inline bool ANIWriter::FileCreate(char* pFileName)
{
    if(mFilePointer != NULL)
    {
        FileClose();
    }
    mFilePointer = fopen(pFileName,"wb");
    if(mFilePointer == NULL)
    {
        return false;
    }
    return true;
}

// Close the file stream
inline bool ANIWriter::FileClose()
{
    if(mFilePointer != NULL)
    {
        fclose(mFilePointer);
        return true;
    }
    return false;
}

// Check whether the file stream is open.
inline bool ANIWriter::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

// Get the file stream options
inline void ANIWriter::GetWriteOptions()
{
}

// Write file with stream options
inline bool ANIWriter::Write(FbxDocument* pDocument)
{
    FbxStatus status;
    if (!pDocument) {
        status.SetCode(FbxStatus::eFailure, "Invalid document handle");
        return false;
    }

    FbxScene* lScene = FbxCast<FbxScene>(pDocument);
    bool lIsAScene = (lScene != NULL);
    bool lResult = false;

    if(lIsAScene)
    {
        lResult = true;
    }
    return lResult;
}

// Pre-process the scene before write it out
inline bool ANIWriter::PreprocessScene(FbxScene &pScene)
{
    printf("I'm in pre-process\n");
    return true;
}

// Post-process the scene after write it out
inline bool ANIWriter::PostprocessScene(FbxScene &pScene)
{
    printf("I'm in post process\n");
    return true;
}

