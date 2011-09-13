git branch --no-color | sed -e "/^[^*]/d" -e "s/* \(.*\)/\1/" > git_branch
set /p gitbranch= < git_branch
del git_branch
for /f "delims=" %%a in ('git describe') do set gittag=%%a
for /f "delims=" %%a in ('git rev-parse HEAD') do set gitrev=%%a
set ver=%gittag%_%gitbranch%_%gitrev%
set timestamp=%date%_%time%
symstore add /r /f Release\*.* /s C:\Symbols /t "InternetRadio" /v "%ver%" /c "%timestamp%"
set out_dir=out
set out_build_dir=%out_dir%\%ver%
if not exist %out_dir% mkdir %out_dir%
if exist %out_build_dir% rmdir /s /q %out_build_dir%
mkdir %out_build_dir%
xcopy Release\InternetRadio.exe %out_build_dir%
xcopy Release\InternetRadio.pdb %out_build_dir%
xcopy dependencies\bass\bass.dll %out_build_dir%
xcopy data\config.json %out_build_dir%
mkdir %out_build_dir%\img
xcopy data\img %out_build_dir%\img