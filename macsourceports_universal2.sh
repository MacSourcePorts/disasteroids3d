# game/app specific values
export APP_VERSION="1.4.0"
export ICONSDIR="."
export ICONSFILENAME="disasteroids3d"
export PRODUCT_NAME="disasteroids3d"
export EXECUTABLE_NAME="disasteroids3d"
export PKGINFO="APPLD3D"
export COPYRIGHT_TEXT="Disasteroids 3D Copyright © 2000 Thom Wetzel, Inc. All rights reserved."

#constants
source ../MSPScripts/constants.sh

rm -rf ${BUILT_PRODUCTS_DIR}

# create folders for make
rm -rf ${X86_64_BUILD_FOLDER}
mkdir ${X86_64_BUILD_FOLDER}

rm -rf ${ARM64_BUILD_FOLDER}
mkdir ${ARM64_BUILD_FOLDER}

# perform builds with make
(ARCH=x86_64 SDL2_INCLUDE=/usr/local/include/SDL2 SDL2_LIB=/usr/local/lib make -j$NCPU)
mkdir -p ${X86_64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}
mv ${X86_64_BUILD_FOLDER}/${EXECUTABLE_NAME} ${X86_64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}

(ARCH=arm64 SDL2_INCLUDE=/opt/homebrew/include/SDL2 SDL2_LIB=/opt/homebrew/lib make -j$NCPU)
mkdir -p ${ARM64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}
mv ${ARM64_BUILD_FOLDER}/${EXECUTABLE_NAME} ${ARM64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}

# create the app bundle
"../MSPScripts/build_app_bundle.sh"

#create any app-specific directories
if [ ! -d "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res" ]; then
	mkdir -p "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res" || exit 1;
fi

cp Res/* "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/Res"

#sign and notarize
"../MSPScripts/sign_and_notarize.sh" "$1"