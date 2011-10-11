for /f "delims=" %%a in ('git describe --tags') do set gittag=%%a
set ver=%gittag%
set timestamp=%date%_%time%
set out_dir=out
set out_build_dir=%out_dir%\%ver%
if not exist %out_dir% mkdir %out_dir%
if exist %out_build_dir% rmdir /s /q %out_build_dir%
mkdir %out_build_dir%
xcopy Win32\Release\InternetRadio.exe %out_build_dir%
xcopy Win32\Release\InternetRadio.pdb %out_build_dir%
xcopy dependencies\bass\bin\Win32\bass.dll %out_build_dir%
xcopy data\config.json %out_build_dir%
mkdir %out_build_dir%\img
xcopy data\img %out_build_dir%\img
cd %out_build_dir%
echo [.] > version
for %%a in (*.*) do (
    if not %%a==version (
        if not %%a==InternetRadio.pdb (
            ..\..\bin\md5sums -u %%a >> version
        )
    )
)
for /d %%a in (*) do (
    echo [%%a] >> version
    for %%b in (%%a\*.*) do (
        ..\..\bin\md5sums -u %%b >> version
    )
)
cd ..\..