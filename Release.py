import subprocess as sub
import datetime as dt
import hashlib
import shutil
import os

def fileMD5(file):
	f = open(file, "rb")
	blockSize = 2**20
	md5 = hashlib.md5()
	while True:
		data = f.read(blockSize)
		if not data:
			break
		md5.update(data)
	f.close()
	return md5.hexdigest()

def releaseForArchitecture(arch):
	sub.Popen("symstore add /r /f " + arch + "\Release\*.* /s \"" + os.environ["SYMBOLPATH"] + "\" /t \"InternetRadio\" /v \"" + gitTag + "\" /c \"" + timeStamp + "\"")
	
	outDirArch = outDirVer + "/" + arch
	binDirArch = arch + "/Release"
	os.mkdir(outDirArch)
	shutil.copy(binDirArch + "/InternetRadio.exe", outDirArch)
	shutil.copy(binDirArch + "/InternetRadio.pdb", outDirArch)
	shutil.copy("dependencies/bass/bin/" + arch + "/bass.dll", outDirArch)
	shutil.copy("data/language.json", outDirArch)
	fChecksums = open(outDirArch + "/checksums", "w")
	for root, dirs, files in os.walk(outDirArch):
		for file in files:
			if file == "checksums":
				continue
			fPath = root + "/" + file
			fSum = fileMD5(fPath)
			fChecksums.write(os.path.relpath(fPath, outDirArch) + ":" + fSum + '\n')
	fChecksums.close()
	
	sub.Popen(os.environ["INNOSETUP"] + "/ISCC /dMyAppVersion=\"" + gitTag + "\" /dMyAppArchitecture=\"" + arch + "\" Setup.iss")

def makeDirDropIfExists(dir):
	if (os.path.exists(dir)):
		shutil.rmtree(dir)
	os.mkdir(dir)

pGitTag = sub.Popen("git describe", shell=True, stdout=sub.PIPE)
gitTag = pGitTag.stdout.readlines()[0].strip()

timeStamp = dt.datetime.now().isoformat('_')

outDir = "publish"
outDirVer = outDir + "/" + gitTag

makeDirDropIfExists(outDirVer)

pGitTagA = sub.Popen("git tag -n50 -l " + gitTag, shell=True, stdout=sub.PIPE)
gitTagA = pGitTagA.stdout.readlines()
fVersion = open(outDirVer + "/changelog", "w")
for gitTagALine in gitTagA:
	if (gitTagALine.find(gitTag) == 0):
		gitTagALine = gitTagALine[len(gitTag):]
	fVersion.write(gitTagALine.strip() + '\n')
fVersion.close()

releaseForArchitecture("Win32")
releaseForArchitecture("x64")

fCurrentVersion = open(outDir + "/version", "w")
fCurrentVersion.write(gitTag)
fCurrentVersion.close()