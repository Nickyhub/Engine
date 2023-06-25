pushd E:\Dev\Projekte\Engine\

echo "assets\shaders\MaterialShader.vert.glsl -> assets\shaders\MaterialShader.vert.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vert Engine\assets\shaders\MaterialShader.vert.glsl -o Engine\assets\shaders\MaterialShader.vert.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "assets\shaders\MaterialShader.frag.glsl -> assets\shaders\MaterialShader.frag.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=frag Engine\assets\shaders\MaterialShader.frag.glsl -o Engine\assets\shaders\MaterialShader.frag.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)


xcopy "Engine\assets" "bin\bin\x64\Debug\assets" /h /i /c /k /e /r /y
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
popd
echo "Done."
PAUSE