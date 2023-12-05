#include "custom.h"
#include "MSHWriter.h"
#include "MSHReader.h"

inline FbxWriter* CreateMSHWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID);
void* GetMSHWriterInfo(FbxWriter::EInfoRequest pRequest, int pId);
void FillMSHWriterIOSettings(FbxIOSettings& pIOS);

inline FbxReader* CreateMSHReader(FbxManager& pManager, FbxImporter& pImporter, int pSubID, int pPluginID);
void *GetMSHReaderInfo(FbxReader::EInfoRequest pRequest, int pId);
void FillMSHReaderIOSettings(FbxIOSettings& pIOS);

// Create your own writer.
// And your writer will get a pPluginID and pSubID.
inline FbxWriter* CreateMSHWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID)
{
    FbxStatus status;
    return new MSHWriter(pManager, pPluginID, status);
}

// Get extension, description or version info about MSHWriter
inline void* GetMSHWriterInfo(FbxWriter::EInfoRequest pRequest, int pId)
{
    static char const* sExt[] =
    {
        "MSH",
        0
    };

    static char const* sDesc[] =
    {
        "Dragon Nest Mesh Format (*.MSH)",
        0
    };

    switch (pRequest)
    {
    case FbxWriter::eInfoExtension:
        return sExt;
    case FbxWriter::eInfoDescriptions:
        return sDesc;
    case FbxWriter::eInfoVersions:
        return 0;
    default:
        return 0;
    }
}

inline void FillMSHWriterIOSettings(FbxIOSettings& pIOS)
{
    // Here you can write your own FbxIOSettings and parse them.
}


// Creates a MSHReader in the Sdk Manager
inline FbxReader* CreateMSHReader(FbxManager& pManager, FbxImporter& pImporter, int pSubID, int pPluginID)
{
    FbxStatus status;
    return new MSHReader(pManager, pPluginID, status);
}

// Get extension, description or version info about MSHReader
inline void *GetMSHReaderInfo(FbxReader::EInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case FbxReader::eInfoExtension:
        return GetMSHWriterInfo(FbxWriter::eInfoExtension, pId);
    case FbxReader::eInfoDescriptions:
        return GetMSHWriterInfo(FbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

inline void FillMSHReaderIOSettings(FbxIOSettings& pIOS)
{
    // Here you can write your own FbxIOSettings and parse them.
}

