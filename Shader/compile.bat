"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E VS /T vs_4_0 /Fo impulsion.vso impulsion.fx
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E GS /T gs_4_0 /Fo impulsion.gso impulsion.fx 
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E PS /T ps_4_0 /Fo impulsion.fso impulsion.fx 

"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E VS /T vs_4_0 /Fo advance.vso advance.fx
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E GS /T gs_4_0 /Fo advance.gso advance.fx 
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E PS /T ps_4_0 /Fo advance.fso advance.fx 

"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E VS /T vs_4_0 /Fo render.vso render.fx
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E GS /T gs_4_0 /Fo render.gso render.fx 
"%DXSDK_DIR%\Utilities\bin\x64\fxc.exe" /E PS /T ps_4_0 /Fo render.fso render.fx 