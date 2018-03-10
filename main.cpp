////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
#include "OcclusionCulling.h"

void ParseCommandLine()
{
	LPTSTR commandLine = GetCommandLineW();
	int numArgs = 0;
	LPWSTR* argv = CommandLineToArgvW(commandLine, &numArgs);

	for(int i = 1; i < numArgs; i += 2) 
	{
		if(!_wcsicmp(argv[i], L"-dropdown1"))
		{
			if(!_wcsicmp(argv[i+1], L"SSE_TYPE"))
			{
				gSOCType = SSE_TYPE;
			}
			else
			{
				gSOCType = SCALAR_TYPE;
			}
		}
		if(!_wcsicmp(argv[i], L"-slider1"))
		{
			gOccluderSizeThreshold = (float)_wtof(argv[i+1]);
		}
		if(!_wcsicmp(argv[i], L"-slider2"))
		{
			gOccludeeSizeThreshold = (float)_wtof(argv[i+1]);
		}
		if(!_wcsicmp(argv[i], L"-slider3"))
		{
			gDepthTestTasks = wcstoul(argv[i+1], NULL, 10);
		}
	}
}

// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	ParseCommandLine();

    // Prevent unused parameter compiler warnings
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    CPUTResult result=CPUT_SUCCESS;
    int returnCode=0;

    // create an instance of my sample
    MySample *pSample = new MySample();
    
    // We make the assumption we are running from the executable's dir in
    // the CPUT SampleStart directory or it won't be able to use the relative paths to find the default
    // resources    
	cString ResourceDirectory;
	CPUTOSServices::GetOSServices()->GetWorkingDirectory(&ResourceDirectory);
	ResourceDirectory.append(_L("\\CPUT\\resources\\"));

    // Initialize the system and give it the base CPUT resource directory (location of GUI images/etc)
    // For now, we assume it's a relative directory from the executable directory.  Might make that resource
    // directory location an env variable/hardcoded later
    pSample->CPUTInitialize(ResourceDirectory); 

    // window and device parameters
    CPUTWindowCreationParams params;
    params.deviceParams.refreshRate         = 0; //60;
    params.deviceParams.swapChainBufferCount= 1;
    params.deviceParams.swapChainFormat     = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    params.deviceParams.swapChainUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	//CPUTOSServices::GetOSServices()->GetDesktopDimensions(&params.windowWidth, &params.windowHeight);

    // parse out the parameter settings or reset them to defaults if not specified
    cString AssetFilename;
    cString CommandLine(lpCmdLine);
    pSample->CPUTParseCommandLine(CommandLine, &params, &AssetFilename);       

    params.windowWidth = SCREENW;
    params.windowHeight = SCREENH;

    // create the window and device context
    result = pSample->CPUTCreateWindowAndContext(_L("CPUTWindow DirectX 11"), params);
    ASSERT( CPUTSUCCESS(result), _L("CPUT Error creating window and context.") );
    
	// initialize the task manager
    gTaskMgr.Init();

    // start the main message loop
    returnCode = pSample->CPUTMessageLoop();

	pSample->DeviceShutdown();

	// shutdown task manage
	gTaskMgr.Shutdown();

    // cleanup resources
    delete pSample;

    return returnCode;
}



