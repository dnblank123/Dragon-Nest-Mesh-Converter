
#include "stdafx.h"
#include <stdio.h>
#include <fbxmanager.h>

namespace f2m
{
    inline void ShowHelp()
    {
        printf("FBX to MSH Converter\t\tby GPBeta\n\n");
        printf("usage: fbx2msh.exe input.fbx output.msh\n");
    }

    int main(int argc, char* argv[])
    {
        if (argc != 3) {
            ShowHelp();
            return 0;
        }

        //initial fbx
        FbxManager* lSdkManager;
        lSdkManager = InitialFbxSdk();

        //paths
        wchar_t* fbxPath = new wchar_t[MAX_PATH];
        wchar_t* mshPath = new wchar_t[MAX_PATH];
        size_t pathSize;

        pathSize = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0, argv[1], -1, fbxPath, pathSize);

        pathSize = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0, argv[2], -1, mshPath, pathSize);

        if (ConvertFile(lSdkManager, fbxPath, 0, NULL, -1, mshPath, 10))
            printf("Convert Succeed\n");
        else
            printf("Convert Failed.Type 'h' for the usage.\n");

        delete fbxPath;
        delete mshPath;

        return 0;
    }
}

