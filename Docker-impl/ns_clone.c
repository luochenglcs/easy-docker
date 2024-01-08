//reference: man clone
#define _GNU_SOURCE
#include <err.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>
#include <getopt.h>
#include <cjson/cJSON.h>
#include <sys/mount.h>
#include <errno.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

static void setContainerHostname(char *hostname)
{
     struct utsname uts;

    /* Change hostname in UTS namespace of child. */
    if (sethostname(hostname, strlen(hostname)) == -1)
        err(EXIT_FAILURE, "sethostname");

    /* Retrieve and display hostname. */
    if (uname(&uts) == -1)
        err(EXIT_FAILURE, "uname");
    printf("uts.nodename in child:  %s\n", uts.nodename);

    return;
}

static void setContainerMnt(char *path)
{
    char old_mnt[64];
#if 0
    printf("mount %s /\n", path);
    if(chroot(path) != 0) {
        printf("chroot failed\n");
    }
    chdir ("/");
#endif
    memset(old_mnt, 0, sizeof(old_mnt));
    sprintf(old_mnt,"%s/old_mnt", path);
    
    //unshare(CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNET);
    unshare(CLONE_NEWNS);
    //
    if (mount("", "/", "" , MS_SLAVE | MS_REC, NULL) == -1)
        printf("mount failed /\n");
    if (mount(path, path, "bind", 0x5000, NULL) == -1)
        printf("mount failed %s\n", path);
    //chroot(path);
    mkdir(old_mnt, 0x755);
    if (syscall(SYS_pivot_root, path, old_mnt) == -1)
        printf("SYS_pivot_root failed\n");

    if (mount("proc", "/proc", "proc", 0xe, 0))
        printf("mount proc failed\n");

    if (umount2("/old_mnt", MNT_DETACH)) 
        printf("umount failed %s\n", strerror(errno));
    rmdir("/old_mnt");
    chdir("/");
}

//reference: man setns
static int parseNsConf(void)
{
    long fileSize;
    FILE *file = fopen("namespace.json", "r");

    if (file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(fileSize + 1);
    fread(buffer, fileSize, 1, file);
    fclose(file);
    buffer[fileSize] = '\0';

    cJSON *jsonObject = cJSON_Parse(buffer);
    free(buffer);
    if (jsonObject == NULL) {
        printf("Failed to parse JSON.\n");
        return 1;
    }

    cJSON *pos;
    //Parse uts namespace config and set
    cJSON *utsField = cJSON_GetObjectItemCaseSensitive(jsonObject, "uts");
    cJSON_ArrayForEach(pos, utsField) {
        if (!strcmp(pos->string, "hostname")) {
            setContainerHostname(pos->valuestring);
        }
        //others TO DO
    }

    cJSON *mntField = cJSON_GetObjectItemCaseSensitive(jsonObject, "mnt");
    cJSON_ArrayForEach(pos, mntField) {
        if (!strcmp(pos->string, "path")) {
            setContainerMnt(pos->valuestring);
        }
    }

    cJSON *runField = cJSON_GetObjectItemCaseSensitive(jsonObject, "RUN");
    cJSON *progrmField = cJSON_GetObjectItemCaseSensitive(runField, "program");
    cJSON *argField = cJSON_GetObjectItemCaseSensitive(runField, "arg");
    char *program = malloc(128);
    memset(program, 0, 128);
    memcpy(program, progrmField->valuestring, strlen(progrmField->valuestring));
    char *arg =NULL;
    if (argField) {
        arg = malloc(128);
        memset(arg, 0, 128);
        memcpy(arg, argField->valuestring, strlen(argField->valuestring));
    }
    cJSON_Delete(jsonObject);

    if (program) {
        printf("%s, %s\n", program, arg);
        execvp(program, &arg);
        printf("%s, %s\n", program, arg);
        printf("execvp failed, err: %s\n", strerror(errno));
    }

    return 0;
}

static int              /* Start function for cloned child */
ContainerFunc(void *unuse)
{
    parseNsConf();
    return 0;           /* Child terminates now */
}

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

int
main(int argc, char *argv[])
{
    char            *stack;         /* Start of stack buffer */
    char            *stackTop;      /* End of stack buffer */
    pid_t           pid;

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        err(EXIT_FAILURE, "mmap");

    stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

    /* Create child that has its own UTS namespace;
        child commences execution in childFunc(). */

    pid = clone(ContainerFunc, stackTop, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, NULL);
    if (pid == -1)
        err(EXIT_FAILURE, "clone");
    printf("clone() returned %jd\n", (intmax_t) pid);

    if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
        err(EXIT_FAILURE, "waitpid");
    printf("child has terminated\n");

    exit(EXIT_SUCCESS);
}
