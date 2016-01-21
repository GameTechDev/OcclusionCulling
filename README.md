Software Occlusion Culling Sample Application
======================================================
The technique used in this sample divides scene objects into occluders and occludees and culls occludees based on a depth comparison with the occluders that are software rasterized to the depth buffer. The sample code uses frustum culling and is optimized with Streaming SIMD Extensions (SSE) and Advanced Vector Extensions (AVX) instruction sets and multi-threading to achieve up to 8X performance speedup compared to a non-culled display of the sample scene.

For a detailed explanation, please see this [article](https://software.intel.com/en-us/articles/software-occlusion-culling).

Build Notes
===========
The sample will not execute properly when run from within Visual Studio unless the 'Working Directory' is set to $(TargetDir).

Requirements
============
- Windows 8 or later
- Visual Studio 2015 or higher


 

