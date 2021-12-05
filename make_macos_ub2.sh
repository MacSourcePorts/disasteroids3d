# game/app specific values
export APP_VERSION="1.4.0"
export ICONSDIR="."
export ICONSFILENAME="disasteroids3d"
export PRODUCT_NAME="disasteroids3d"
export EXECUTABLE_NAME="disasteroids3d"
export PKGINFO="APPLD3D"
export COPYRIGHT_TEXT="Disasteroids 3D Copyright © 2000 Thom Wetzel, Inc. All rights reserved."

# constants
export BUILT_PRODUCTS_DIR="release"
export WRAPPER_NAME="${PRODUCT_NAME}.app"
export CONTENTS_FOLDER_PATH="${WRAPPER_NAME}/Contents"
export EXECUTABLE_FOLDER_PATH="${CONTENTS_FOLDER_PATH}/MacOS"
export UNLOCALIZED_RESOURCES_FOLDER_PATH="${CONTENTS_FOLDER_PATH}/Resources"
export ICONS="${ICONSFILENAME}.icns"
export BUNDLE_ID="com.macsourceports.${PRODUCT_NAME}"

# For parallel make on multicore boxes...
NCPU=`sysctl -n hw.ncpu`

# create makefiles with cmake
rm -rf build-x86_64
mkdir build-x86_64

rm -rf build-arm64
mkdir build-arm64

# perform builds with make
(ARCH=x86_64 make -j$NCPU)
(ARCH=arm64 make -j$NCPU)

# create the app bundle
"../MSPScripts/build_app_bundle.sh"

#create any app-specific directories
if [ ! -d "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res" ]; then
	mkdir -p "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res" || exit 1;
fi

#lipo the executable
lipo build-x86_64/${EXECUTABLE_NAME} build-arm64/${EXECUTABLE_NAME} -output "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}" -create

cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libSDL2-2.0.0.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libSDL2_mixer-2.0.0.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"

cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libmodplug.1.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libvorbisfile.3.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libvorbis.0.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libFLAC.8.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libmpg123.0.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"
cp /Users/tomkidd/Documents/GitHub/MSPStore/lib/libogg.0.dylib "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}"

cp Res/* "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res"

# use install_name tool to point executable to bundled resources (probably wrong long term way to do it)
#modify x86_64
install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}
install_name_tool -change /usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib @executable_path/libSDL2_mixer-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}

install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib

install_name_tool -change /usr/local/opt/libmodplug/lib/libmodplug.1.dylib @executable_path/libmodplug.1.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbisfile.3.dylib @executable_path/libvorbisfile.3.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/flac/lib/libFLAC.8.dylib @executable_path/libFLAC.8.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/mpg123/lib/libmpg123.0.dylib @executable_path/libmpg123.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib

install_name_tool -change /usr/local/Cellar/libvorbis/1.3.7/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbisfile.3.dylib
install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbisfile.3.dylib

install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbis.0.dylib

install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libFLAC.8.dylib

#modify arm64
install_name_tool -change /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}
install_name_tool -change /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib @executable_path/libSDL2_mixer-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}

install_name_tool -change /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2-2.0.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib

install_name_tool -change /opt/homebrew/opt/libmodplug/lib/libmodplug.1.dylib @executable_path/libmodplug.1.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /opt/homebrew/opt/libvorbis/lib/libvorbisfile.3.dylib @executable_path/libvorbisfile.3.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /opt/homebrew/opt/libvorbis/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /opt/homebrew/opt/flac/lib/libFLAC.8.dylib @executable_path/libFLAC.8.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /opt/homebrew/opt/mpg123/lib/libmpg123.0.dylib @executable_path/libmpg123.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libSDL2_mixer-2.0.0.dylib

install_name_tool -change /opt/homebrew/Cellar/libvorbis/1.3.7/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbisfile.3.dylib
install_name_tool -change /opt/homebrew/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbisfile.3.dylib

install_name_tool -change /opt/homebrew/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libvorbis.0.dylib

install_name_tool -change /opt/homebrew/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib ${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/libFLAC.8.dylib

echo "bundle done."

#sign and notarize
"../MSPScripts/sign_and_notarize.sh" "$1"