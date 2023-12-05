// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

// Do not define WIN32_LEAN_AND_MEAN to use OPENFILENAME structure
// #define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <locale.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <fbxsdk.h>
#include "translate.h"
#include "Resource/resource.h"
#include "MSHIOPlugin/MSHIOPlugin.h"
#include "ANIIOPlugin/ANIIOPlugin.h"

#define STATUS_WINDOWS 100
#define CONVERT_BUTTON 1001
#define IMPORT_FILE_BUTTON 1002
#define IMPORT_ANI_BUTTON 1003
#define IMPORT_FILE_EDITBOX 1004
#define IMPORT_ANI_EDITBOX 1005
#define MESH_DATA_EDITBOX 1006
#define BONE_DATA_EDITBOX 1007
#define BIND_DATA_EDITBOX 1008
#define OTHER_DATA_EDITBOX 1009

//自定义函数声明
inline const wchar_t* GetIOFilters(FbxManager*, int);
inline void GetFileName(HWND, FbxManager*, wchar_t*,int &, int);
inline bool ConvertFile(FbxManager* , wchar_t*, int, wchar_t*, int, wchar_t*, int);
inline FbxManager* InitialFbxSdk();


//获取文件格式filter
inline const wchar_t* GetIOFilters(FbxManager* pSdkManager, int ioSettings)
{
    FbxString s;
    int filterCount = 0;
    if(ioSettings == 0){ //获取Reader支援
        filterCount = pSdkManager->GetIOPluginRegistry()->GetReaderFormatCount();
        for(int i = 0; i < filterCount - 1; i++){   //减1为去掉ANI读取列表
            s += pSdkManager->GetIOPluginRegistry()->
                GetReaderFormatDescription(i);
            s += "|*.";
            s += pSdkManager->GetIOPluginRegistry()->
                GetReaderFormatExtension(i);
            s += "|";
        }
    }
    else{
        filterCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();
        for(int i = 0; i < filterCount; i++){
            s += pSdkManager->GetIOPluginRegistry()->
                GetWriterFormatDescription(i);
            s += "|*.";
            s += pSdkManager->GetIOPluginRegistry()->
                GetWriterFormatExtension(i);
            s += "|";
        }
    }

    //用'\0'替换'|'
    int nbChar   = int(strlen(s.Buffer())) + 1;
    wchar_t *filter = new wchar_t[nbChar];
    ZeroMemory(filter, nbChar);
    MultiByteToWideChar(CP_ACP, NULL, s.Buffer(), -1, filter, nbChar);
    for(int i = 0; i < nbChar; i++)
        if(filter[i] == '|')
            filter[i] = '\0';

    //调用函数需自行删除filter
    return filter;
}

//ioSettings:
//0 - inputFile
//1 - aniFile
//2 - outputFile
inline void GetFileName(HWND hwnd,
                 FbxManager* pSdkManager,
                 wchar_t* szFile,
                 int &formatNum,
                 int ioSettings)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ZeroMemory(szFile, MAX_PATH);

    //初始化文件选区框
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = szFile;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.nFilterIndex    = 1;
    ofn.lpstrInitialDir = NULL;

    const wchar_t *filter  = GetIOFilters(pSdkManager, ioSettings);

    switch(ioSettings){
        case 0:
            ofn.lpstrFilter     = filter;
            ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            ofn.nFilterIndex    = 6;
            GetOpenFileName(&ofn);  //显示文件选择框
            formatNum = ofn.nFilterIndex - 1;
            break;
        case 1:
            ofn.lpstrFilter     = L"Dragon Nest Animate Format (*.ANI)\0*.ANI\0\0";
            ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            GetOpenFileName(&ofn);
            formatNum = -1;
            break;
        default:
            ofn.lpstrFilter     = filter;
            ofn.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
            GetSaveFileName(&ofn);
            formatNum = ofn.nFilterIndex - 1;
    }

    delete filter;
}

inline bool ConvertFile(FbxManager* pSdkManager,
                 wchar_t* inputFile, int inputFormat,
                 wchar_t* aniFile, int aniFormat,
                 wchar_t* outputFile, int outputFormat)
{
    char tempPath[MAX_PATH];
    bool lResult = false;
    int lRegisteredCount;
    int lPluginId;
    FbxScene* lScene = FbxScene::Create(pSdkManager,"");

    //创建Importer/Exporter
    FbxImporter* lImporter = FbxImporter::Create(pSdkManager,"");
    FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

    //导入文件
    if(inputFormat == 5)    //MSH (ANSI)
        WideCharToMultiByte(CP_ACP, NULL, inputFile, -1, tempPath, MAX_PATH, NULL, NULL);
    else    //OTHERS (UTF-8)
        WideCharToMultiByte(CP_UTF8, NULL, inputFile, -1, tempPath, MAX_PATH, NULL, NULL);
    lResult = lImporter->Initialize(tempPath, inputFormat, pSdkManager->GetIOSettings());
    if(!lResult)return false;
    lResult = lImporter->Import(lScene);
    if(!lResult)return false;

    //导入ANI
    if(inputFormat == 5 && *aniFile){    //导入MSH时需求ANI
        WideCharToMultiByte(CP_ACP, NULL, aniFile, -1, tempPath, MAX_PATH, NULL, NULL);
        lResult = lImporter->Initialize(tempPath, aniFormat, pSdkManager->GetIOSettings());
        if(!lResult)return false;
        lResult = lImporter->Import(lScene);
        if(!lResult)return false;
    }

    //导出文件
    if(outputFormat == 10)   //MSH (ANSI)
        WideCharToMultiByte(CP_ACP, NULL, outputFile, -1, tempPath, MAX_PATH, NULL, NULL);
    else    //OTHERS (UTF-8)
        WideCharToMultiByte(CP_UTF8, NULL, outputFile, -1, tempPath, MAX_PATH, NULL, NULL);
    lResult = lExporter->Initialize(tempPath, outputFormat, pSdkManager->GetIOSettings());
    if(!lResult)return false;
    lResult = lExporter->Export(lScene);
    if(!lResult)return false;
    if(pSdkManager)
        pSdkManager->Destroy();
    ZeroMemory(outputFile, MAX_PATH);

    return lResult;
}

inline FbxManager* InitialFbxSdk()
{
    //创建FBXSDK管理器
    int lRegisteredCount;
    int lPluginId;
    FbxManager* lSdkManager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);
    //注册组件
    lSdkManager->GetIOPluginRegistry()->RegisterReader(CreateMSHReader, GetMSHReaderInfo,
        lPluginId, lRegisteredCount, FillMSHReaderIOSettings);
    lSdkManager->GetIOPluginRegistry()->RegisterWriter(CreateMSHWriter, GetMSHWriterInfo,
        lPluginId, lRegisteredCount, FillMSHWriterIOSettings);
    lSdkManager->GetIOPluginRegistry()->RegisterReader(CreateANIReader, GetANIReaderInfo,
        lPluginId, lRegisteredCount, FillANIReaderIOSettings);
    return lSdkManager;
}
