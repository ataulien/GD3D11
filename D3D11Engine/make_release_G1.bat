echo OFF
set p=%CD%

echo Creating ZIP for G1...

del ..\PreviewBuilds\GD3D11_PreviewReleaseG1.zip
cd /d %G1_SYSTEM_PATH%
CALL 7z a -tzip %p%\GD3D11_PreviewReleaseG1.zip @%p%\make_release_listfile.txt
echo %p%
cd /d %p%
 
move GD3D11_PreviewReleaseG1.zip ..\PreviewBuilds\GD3D11_PreviewReleaseG1.zip