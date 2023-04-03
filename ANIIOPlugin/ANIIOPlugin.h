#include "custom.h"
#include "ANIReader.h"
#include "ANIWriter.h"

KFbxWriter* CreateANIWriter(KFbxSdkManager& pManager, KFbxExporter& pExporter, int pSubID, int pPluginID);
void* GetANIWriterInfo(KFbxWriter::KInfoRequest pRequest, int pId);
void FillANIWriterIOSettings(KFbxIOSettings& pIOS);

KFbxReader* CreateANIReader(KFbxSdkManager& pManager, KFbxImporter& pImporter, int pSubID, int pPluginID);
void *GetANIReaderInfo(KFbxReader::KInfoRequest pRequest, int pId);
void FillANIReaderIOSettings(KFbxIOSettings& pIOS);

// Create your own writer.
// And your writer will get a pPluginID and pSubID.
KFbxWriter* CreateANIWriter(KFbxSdkManager& pManager, KFbxExporter& pExporter, int pSubID, int pPluginID)
{
    return FbxSdkNew<ANIWriter>(pManager, pPluginID);
}

// Get extension, description or version info about ANIWriter
void* GetANIWriterInfo(KFbxWriter::KInfoRequest pRequest, int pId)
{
    static char const* sExt[] =
    {
        "ANI",
        0
    };

    static char const* sDesc[] =
    {
        "Dragon Nest Animate Format (*.ANI)",
        0
    };

    switch (pRequest)
    {
    case KFbxWriter::eInfoExtension:
        return sExt;
    case KFbxWriter::eInfoDescriptions:
        return sDesc;
    case KFbxWriter::eInfoVersions:
        return 0;
    default:
        return 0;
    }
}

void FillANIWriterIOSettings(KFbxIOSettings& pIOS)
{
    // Here you can write your own KFbxIOSettings and parse them.
}

// Creates a ANIReader in the Sdk Manager
KFbxReader* CreateANIReader(KFbxSdkManager& pManager, KFbxImporter& pImporter, int pSubID, int pPluginID)
{
    return FbxSdkNew<ANIReader>(pManager, pPluginID);
}

// Get extension, description or version info about ANIReader
void *GetANIReaderInfo(KFbxReader::KInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case KFbxReader::eInfoExtension:
        return GetANIWriterInfo(KFbxWriter::eInfoExtension, pId);
    case KFbxReader::eInfoDescriptions:
        return GetANIWriterInfo(KFbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

void FillANIReaderIOSettings(KFbxIOSettings& pIOS)
{
    // Here you can write your own KFbxIOSettings and parse them.
}

