
#need cjson-devel
default: ns_clone.c ns_exec.c
	gcc -g ns_clone.c -lcjson -o ns_clone
	gcc -g ns_exec.c -o ns_exec

clean:
	rm -rf ns_clone
	rm -rf ns_exec
