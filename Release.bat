git branch --no-color | sed -e "/^[^*]/d" -e "s/* \(.*\)/\1/" > git_branch
set /p gitbranch= < git_branch
del git_branch
for /f "delims=" %%a in ('git describe') do set gittag=%%a
for /f "delims=" %%a in ('git rev-parse HEAD') do set gitrev=%%a
set ver=%gittag%_%gitbranch%_%gitrev%
set timestamp=%date%_%time%
symstore add /r /f Release\*.* /s C:\Symbols /t "InternetRadio" /v "%ver%" /c "%timestamp%"
if exist out rmdir /s /q out
mkdir out
xcopy Release\InternetRadio.exe out
xcopy Release\InternetRadio.pdb out
xcopy dependencies\bass\bass.dll out
xcopy data\config.json out
mkdir out\img
xcopy data\img out\img