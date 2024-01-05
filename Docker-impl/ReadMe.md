依赖包：cjson-devel
编译： gcc -g ns_clone.c -lcjson -o ns_clone
解压个镜像：
```c
docker pull busybox
docker export -o busybox.tar `docker run --name test -d busybox top -b`
docker stop test
tar -axvf busybox.tar
```
