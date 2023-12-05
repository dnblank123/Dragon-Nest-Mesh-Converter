
#include "stdafx.h"
#include <stdio.h>
#include <fbxmanager.h>
#pragma comment (lib, "libfbxsdk.lib")
#pragma comment(lib, "comctl32.lib")

inline void ShowHelp()
{
    printf("MSH to FBX Converter\t\tby GPBeta\n\n");
    printf("usage: msh2fbx.exe input.msh input.ani output.fbx\n");
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        ShowHelp();
        return 0;
    }

    //initial fbx
    FbxManager* lSdkManager;
    lSdkManager = InitialFbxSdk();

    //paths
    wchar_t* mshPath = new wchar_t[MAX_PATH];
    wchar_t* aniPath = new wchar_t[MAX_PATH];
    wchar_t* fbxPath = new wchar_t[MAX_PATH];
    size_t pathSize;

    pathSize = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, mshPath, pathSize);

    pathSize = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0, argv[2], -1, aniPath, pathSize);

    pathSize = MultiByteToWideChar(CP_ACP, 0, argv[3], -1, NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0, argv[3], -1, fbxPath, pathSize);

    if (ConvertFile(lSdkManager, mshPath, 5, aniPath, -1, fbxPath, 1))
        printf("Convert Succeed\n");
    else
        printf("Convert Failed.Type 'h' for the usage.\n");

    delete fbxPath;
    delete mshPath;
    delete aniPath;

    return 0;
}


