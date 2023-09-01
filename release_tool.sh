#!/bin/bash
set -e
echo=echo

# Set Color
CSI=$($echo -e "\033[")
CEND="${CSI}0m"
CDGREEN="${CSI}32m"
CRED="${CSI}1;31m"
CGREEN="${CSI}1;32m"
CYELLOW="${CSI}1;33m"
CBLUE="${CSI}1;34m"
CMAGENTA="${CSI}1;35m"
CCYAN="${CSI}1;36m"
CSUCCESS="$CDGREEN"
CFAILURE="$CRED"
CQUESTION="$CMAGENTA"
CWARNING="$CYELLOW"
CMSG="$CCYAN"

function remove_dir() {
    local dir=$1
    if [ -d "$dir" ]; then
        rm -rf $dir
        if [ $? -ne 0 ]; then
            echo "${CFAILURE}remove dir: [$dir] failed${CEND}"
            return 1
        fi
    fi
}

function new_dir() {
    local dir=$1
    remove_dir $dir
    mkdir -p $dir
}

function get_version_num() {
    VERSION=$1
    local num=$(cat $VERSION_FILE | grep -i ${VERSION} | grep -oE "[0-9]+")
    echo $num
}


PWD=$(dirname $(realpath "$0"))
CURRENT_DIR=$PWD
nproc=$(nproc)

VERSION_FILE=${CURRENT_DIR}/helper/cmake/version.cmake

./build.sh

# 打包
PKG=${CURRENT_DIR}/pkg
new_dir ${PKG}

cd ${PKG}
rc=$?

COMMIT=$(git rev-parse --short HEAD)
VERSION_MAJOR=$(get_version_num VERSION_MAJOR)
VERSION_MINOR=$(get_version_num VERSION_MINOR)
VERSION_PATCH=$(get_version_num VERSION_PATCH)
VERSION_STRING=v$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH-$COMMIT

# 打包文件名称
DATE=`date '+%Y%m%d'`
PKG_NAME=type4_gaze_online_testtool_${VERSION_STRING}_${DATE}
new_dir ${PKG_NAME}

cp -r ${CURRENT_DIR}/build/*online $PKG_NAME
cp -r ${CURRENT_DIR}/architecture $PKG_NAME
cp -r ${CURRENT_DIR}/config*.json $PKG_NAME
cp -r ${CURRENT_DIR}/release_README.md $PKG_NAME/README.md


tar zcvf $PKG_NAME.tar.gz $PKG_NAME && echo -e "${CSUCCESS}build ${PKG_NAME} success ${CEND}"
