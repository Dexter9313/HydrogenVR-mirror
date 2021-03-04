#!/bin/bash

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -yq build-essential wget cmake git qtbase5-dev libassimp-dev libqt5opengl5-dev qtdeclarative5-dev libqt5svg5-dev qttools5-dev libqt5webkit5-dev qtmultimedia5-dev libqt5xmlpatterns5-dev libpython2.7-dev
if [ ! -f "./build.conf" ]; then
        . ./build.conf.example ;
else
    . ./build.conf ;
fi

mkdir deps ; cd deps
# install openvr
OPENVR_COMMIT=0eb70325a4ea3edca2a0ff8aefa631f2529e6eb7 ;
wget --content-disposition https://github.com/ValveSoftware/openvr/raw/$OPENVR_COMMIT/bin/linux64/libopenvr_api.so ;
wget --content-disposition https://raw.githubusercontent.com/ValveSoftware/openvr/$OPENVR_COMMIT/headers/openvr.h ;
cp ./libopenvr_api.so /usr/lib ;
cp ./openvr.h /usr/include ;
# install leap motion sdk
wget --content-disposition https://warehouse.leapmotion.com/apps/4185/download ;
tar xzf Leap_Motion_SDK_Linux_*.tgz ;
cp ./LeapDeveloperKit*/LeapSDK/include/Leap*.h /usr/include ;
cp ./LeapDeveloperKit*/LeapSDK/lib/x64/libLeap.so /usr/lib ;
cd ..
    # install project additional deps
./$PROJECT_DIRECTORY/ci/travis/install.sh
