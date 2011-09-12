git branch --no-color | sed -e "/^[^*]/d" -e "s/* \(.*\)/\1/" > git_branch
set /p gitbranch= < git_branch
del git_branch
for /f "delims=" %%a in ('git describe') do set gittag=%%a
for /f "delims=" %%a in ('git rev-parse HEAD') do set gitrev=%%a
set ver=%gittag%_%gitbranch%_%gitrev%
set timestamp=%date%_%time%
symstore add /r /f Release\*.* /s C:\Symbols /t "InternetRadio" /v "%ver%" /c "%timestamp%"
set dir=%ver%
if not exist out mkdir out
if exist out\%dir% rmdir /s /q out\%dir%
mkdir out\%dir%
xcopy Release\InternetRadio.exe out\%dir%
xcopy Release\InternetRadio.pdb out\%dir%
xcopy dependencies\bass\bass.dll out\%dir%
xcopy data\config.json out\%dir%
mkdir out\%dir%\img
xcopy data\img out\%dir%\img