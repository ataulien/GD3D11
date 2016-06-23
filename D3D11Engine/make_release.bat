echo OFF
set p=%CD%

echo Creating ZIP for G2...

del ..\PreviewBuilds\GD3D11_PreviewReleaseG2.zip
cd /d %G2_SYSTEM_PATH%
CALL 7z a -tzip %p%\GD3D11_PreviewReleaseG2.zip @%p%\make_release_listfile.txt
echo %p%
cd /d %p%
 
move GD3D11_PreviewReleaseG2.zip ..\PreviewBuilds\GD3D11_PreviewReleaseG2.zip