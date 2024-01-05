#!/bin/bash

ls /bin/docker >/dev/null || yum install docker
docker pull $1
mkdir root
cd root
docker export -o $1.tar `docker run --name test -d $1 top -b`
docker stop test
docker remove test
tar -axvf $1.tar
rm -rf $1.tar
cd ..
