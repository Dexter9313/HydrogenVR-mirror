#!/bin/bash

./ci/gitlab-ci/commons/install_dependencies.sh
# latest cmake
apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common wget
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
apt-add-repository -y 'deb https://apt.kitware.com/ubuntu/ bionic main'
apt-get purge -y --auto-remove cmake
apt-get install -y cmake libpythonqt-qt5-python2-dev libpythonqt-qtall-qt5-python2-dev
./ci/gitlab-ci/commons/main_build.sh
