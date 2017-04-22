/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTOSServicesWin_H__
#define __CPUTOSServicesWin_H__



#include "CPUT.h"

// OS includes
#include <windows.h>
#include <errno.h>  // file open error codes
#include <string>   // wstring



class CPUTOSServices
{
public:
    static CPUTOSServices* GetOSServices();
    static CPUTResult DeleteOSServices();

    // screen/window dimensions
    void GetClientDimensions( int *pWidth, int *pHeight);
    void GetClientDimensions( int *pX, int *pY, int *pWidth, int *pHeight);
    void GetDesktopDimensions(int *pWidth, int *pHeight);
    bool IsWindowMaximized();
    bool IsWindowMinimized();
    bool DoesWindowHaveFocus();

    // Mouse capture - 'binds'/releases all mouse input to this window
    void CaptureMouse();
    void ReleaseMouse();
    

    //Working directory manipulation
    CPUTResult GetWorkingDirectory(cString *pPath);
    CPUTResult SetWorkingDirectory(const cString &path);
    CPUTResult GetExecutableDirectory(cString *pExecutableDir);

    // Path helpers
    CPUTResult ResolveAbsolutePathAndFilename(const cString &fileName, cString *pResolvedPathAndFilename);
    CPUTResult SplitPathAndFilename(const cString &sourceFilename, cString *pDrive, cString *pDir, cString *pfileName, cString *pExtension);

    // file handling
    CPUTResult DoesFileExist(const cString &pathAndFilename);
    CPUTResult DoesDirectoryExist(const cString &path);
    CPUTResult OpenFile(const cString &fileName, FILE **pFilePointer);
    CPUTResult ReadFileContents(const cString &fileName, UINT *psizeInBytes, void **ppData);

    // File dialog box
    CPUTResult OpenFileDialog(const cString &filter, cString *pfileName);

    // Informational Message box
    CPUTResult OpenMessageBox(cString title, cString text);

    // error handling
    inline void Assert(bool bCondition) {assert(bCondition);}
    void OutputConsoleString(cString &outputString);
    CPUTResult TranslateFileError(errno_t err);

    // hwnd setup
    inline void SethWnd(const HWND hWnd) { mhWnd = hWnd; };
    inline void GetWindowHandle(HWND *phWnd) { *phWnd = mhWnd; };

    // special keys
    bool ControlKeyPressed(CPUTKey &key);



private:
    CPUTOSServices();
     ~CPUTOSServices();
    static CPUTOSServices *mpOSServices;   // singleton object
    HWND                   mhWnd;
    cString                mCPUTResourceDirectory;
    bool                   FileFoundButWithError(CPUTResult result);

#ifdef CPUT_GPA_INSTRUMENTATION
public:
    // GPA instrumentation (only available in Profile build)
    void GetInstrumentationPointers(__itt_domain **ppGPADomain, CPUT_GPA_INSTRUMENTATION_STRINGS eString, __itt_string_handle **ppGPAStringHandle);
    void SetInstrumentationPointers(__itt_domain *pGPADomain, CPUT_GPA_INSTRUMENTATION_STRINGS eString, __itt_string_handle *pGPAStringHandle);

private:
    // GPA instrumentation member variables
    __itt_domain *mpGPADomain;
    __itt_string_handle *mppGPAStringHandles[GPA_HANDLE_STRING_ENUMS_SIZE];
#endif

};
#endif // __CPUTOSServicesWin_H__
