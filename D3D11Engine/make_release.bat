echo OFF
echo Creating ZIP for G2...
set p=%CD%

del ..\PreviewBuilds\GD3D11_PreviewReleaseG2.zip
cd %G2_SYSTEM_PATH%
CALL 7z a -tzip %p%\GD3D11_PreviewReleaseG2.zip @%p%\make_release_listfile.txt
echo %p%
cd %p%

echo Creating ZIP for G1...

REM del ..\PreviewBuilds\GD3D11_PreviewReleaseG1.zip
REM cd /d %G1_SYSTEM_PATH%
REM CALL 7z a -tzip -mm=LZMA -mx=9 %p%\GD3D11_PreviewReleaseG1.zip @%p%\make_release_listfile.txt
REM echo %p%
REM cd /d %p%
 
move GD3D11_PreviewReleaseG2.zip ..\PreviewBuilds\GD3D11_PreviewReleaseG2.zip
REM move GD3D11_PreviewReleaseG1.zip ..\PreviewBuilds\GD3D11_PreviewReleaseG1.zip