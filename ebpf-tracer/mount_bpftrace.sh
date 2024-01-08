bpftrace -e 'tracepoint:syscalls:sys_enter_mount {printf("dev:%s dir:%s type:%s flags:%lx %lx\n", str(args->dev_name), str(args->dir_name), str(args->type), args->flags, args->dev_name); }'
