

echo "Enter version: (ex: 0.55b)"  
read versionInput
export version=v${versionInput}

rm -rf build-release

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="x86_64"
xtplayerSource="${home}"git/XTPlayer
xtengineSource="${home}"git/XTEngine
xtplayerBuildDirectory="${xtplayerSource}"/build-release
xtengineBuildDirectory="${xtengineSource}"/build-release
xtplayerLocation="${xtplayerBuildDirectory}"/release/
xtengineLocation="${xtengineBuildDirectory}"/release/
deployDirectory="${xtplayerSource}"/Deploy/
qtDirectory="${home}"Qt/6.9.1/macos/bin
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
xtools=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin#Export directory with qmake to path.
export PATH="${qtDirectory}":$xtools:$PATH
export LD_LIBRARY_PATH="${deployDirectory}lib;${xtengineLocation}"

cd ${xtplayerSource}
#if needToPull; then
	git pull || { echo 'git pull XTPlayer failed' ; exit 1; }
#fi
cd ${xtengineSource}
#if needToPull; then
	git pull || { echo 'git pull XTEngine failed' ; exit 1; };
#fi

mkdir -p "${xtengineBuildDirectory}" || { echo 'Make XTEngine build dir failed' ; exit 1; }
cd ${xtengineBuildDirectory}
qmake ${xtengineSource}/src/XTEngine.pro CONFIG+=release || { echo 'QMake XTEngine failed' ; exit 1; }
make Makefile qmake_all || { echo 'Make Makefile failed' ; exit 1; }
make --silent -j24 || { echo 'Make failed' ; exit 1; }
mkdir -p "${xtplayerBuildDirectory}" || { echo 'Make XTPlayer build dir failed' ; exit 1; }
cd ${xtplayerBuildDirectory}
qmake ${xtplayerSource}/src/XTPlayer.pro CONFIG+=release || { echo 'QMake XTPlayer failed' ; exit 1; }
make Makefile qmake_all || { echo 'Make Makefile failed' ; exit 1; }
make --silent -j24 || { echo 'Make failed' ; exit 1; }

#version="v0.54b"
outname="XTPlayer-${version}_MacOS_Universal"
appLocation="${xtplayerLocation}XTPlayer.app"


cd ${xtplayerBuildDirectory}
~/Qt/6.9.1/macos/bin/macdeployqt ${appLocation}
cp -R ../src/themes ${appLocation}/Contents/MacOS/
cp -R ../src/images ${appLocation}/Contents/MacOS/
mkDir ${appLocation}/Contents/MacOS/www
cp ../../XTEngine/src/www/*-min.html ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.js ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.css ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/favicon.ico ${appLocation}/Contents/MacOS/www/favicon.ico
cp ../../XTEngine/build-release/release/lib* ${appLocation}/Contents/Frameworks/
install_name_tool -change libxtengine.1.0.0.dylib @executable_path/../Frameworks/libxtengine.1.0.0.dylib ${appLocation}/Contents/MacOS/XTPlayer
mkdir ${outname}
cp -R ${appLocation} ${outname}/
hdiutil create /tmp/tmp.dmg -ov -volname ${outname} -fs HFS+ -srcfolder ${outname}
hdiutil convert /tmp/tmp.dmg -format UDZO -o ${outname}.dmg
