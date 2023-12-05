#include "custom.h"
#include "ANIReader.h"
#include "ANIWriter.h"

inline FbxWriter* CreateANIWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID);
void* GetANIWriterInfo(FbxWriter::EInfoRequest pRequest, int pId);
void FillANIWriterIOSettings(FbxIOSettings& pIOS);

inline FbxReader* CreateANIReader(FbxManager& pManager, FbxImporter& pImporter, int pSubID, int pPluginID);
void *GetANIReaderInfo(FbxReader::EInfoRequest pRequest, int pId);
void FillANIReaderIOSettings(FbxIOSettings& pIOS);

// Create your own writer.
// And your writer will get a pPluginID and pSubID.
inline FbxWriter* CreateANIWriter(FbxManager& pManager, FbxExporter& pExporter, int pSubID, int pPluginID)
{
    FbxStatus status;
    return new ANIWriter(pManager, pPluginID, status);
}

// Get extension, description or version info about ANIWriter
inline void* GetANIWriterInfo(FbxWriter::EInfoRequest pRequest, int pId)
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

inline void FillANIWriterIOSettings(FbxIOSettings& pIOS)
{
    // Here you can write your own FbxIOSettings and parse them.
}

// Creates a ANIReader in the Sdk Manager
inline FbxReader* CreateANIReader(FbxManager& pManager, FbxImporter& pImporter, int pSubID, int pPluginID)
{
    FbxStatus status;
    return new ANIReader(pManager, pPluginID, status);
}

// Get extension, description or version info about ANIReader
inline void *GetANIReaderInfo(FbxReader::EInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case FbxReader::eInfoExtension:
        return GetANIWriterInfo(FbxWriter::eInfoExtension, pId);
    case FbxReader::eInfoDescriptions:
        return GetANIWriterInfo(FbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

inline void FillANIReaderIOSettings(FbxIOSettings& pIOS)
{
    // Here you can write your own FbxIOSettings and parse them.
}

