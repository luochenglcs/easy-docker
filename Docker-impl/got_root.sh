#!/bin/bash

Tag=${1:-'ubuntu'}
ls /bin/docker >/dev/null || yum install docker
docker pull $Tag
rm -rf root
mkdir root
cd root
docker export -o $Tag.tar `docker run --name test -d $Tag top -b`
docker stop test
docker remove test
tar -axvf $Tag.tar
rm -rf $Tag.tar
cd ..
