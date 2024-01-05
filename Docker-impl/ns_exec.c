//reference: man setns
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>
#include <getopt.h>

//reference: man setns
static struct option opts[] = {
    {"cgroup", 1, 0, 'c'},
	{"ipc", 1, 0, 'i' },
	{"net", 1, 0, 'n' },
	{"mnt", 1, 0, 'm' },
	{"pid", 1, 0, 'p' },
	{"time", 1, 0, 't' },
	{"user", 1, 0, 's' },
	{"uts", 1, 0, 'u'},
	{"all", 1, 0, 'a' },
    {0}
};

static void usage(void)
{
    printf("TO DO!!");
}

int mnt_fd = -1;
#define array_len(x) (sizeof(x)/sizeof(*(x)))
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE);\
                        } while (0)

static int setnsfile(char* file, int nstype)
{
    int fd;

    fd = open(file, O_RDONLY);
    if (fd == -1) {
        printf("%s open %s failed: errno:%s\n", __func__, file, strerror(errno));
        return -1;
    }

    if (setns(fd, 0) == -1) {
         printf("%s setns %s failed: errno:%s\n", __func__, file, strerror(errno));
         return -2;
    }

    return 0;
}

static int setnspath(char *path, int nstype)
{
    struct dirent *dp = NULL;
    char file[64];
    DIR *d = NULL;
    int ret = 0;

    if(!(d = opendir(path))) {
        printf("opendir[%s] error: %s\n", path, strerror(errno));
        return -1;
    }

    while((dp = readdir(d)) != NULL) {
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)) || (!strncmp(dp->d_name, "mnt", 3)))
            continue;
        memset(file, 0, sizeof(file));
        sprintf(file,"%s/%s", path, dp->d_name);
        ret = setnsfile(file, 0);
    }
    memset(file, 0, sizeof(file));
    sprintf(file,"%s/mnt", path);
    mnt_fd = open(file, O_RDONLY);
    if (mnt_fd == -1) {
        printf("%s open %s failed: errno:%s\n", __func__, file, strerror(errno));
        ret = -2;
    }

    closedir(d);
    return 0;
}

static void get_short_opts(struct option *o, char *s)
{
	*s++ = '+';
	while (o->name) {
		if (isprint(o->val)) {
			*s++ = o->val;
			if (o->has_arg)
				*s++ = ':';
		}
		o++;
	}
	*s = '\0';
}


int
main(int argc, char *argv[])
{
    int c;
	char shortopts[array_len(opts)*2 + 1];

	get_short_opts(opts,shortopts);
	while ((c = getopt_long(argc, argv, shortopts, opts, NULL)) != -1) {
        switch(c) {
        case 'c':
        case 'i':
        case 'n':
        case 'p':
        case 't':
        case 's':
        case 'u':
            setnsfile(optarg, 0);
            break;
        case 'm':
            mnt_fd = open(optarg, O_RDONLY);
            if (mnt_fd == -1) {
                printf("%s open %s failed: errno:%s\n", __func__, optarg, strerror(errno));
                return -1;
            }
            break;
        case 'a':
            printf("hello\n");
            printf("%s\n", optarg);
            setnspath(optarg, 0);
            break;
        default:
            usage();
        }
    }

    if (mnt_fd != -1) {
        if (setns(mnt_fd, 0) == -1) {
            printf("%s setns %s failed: errno:%s\n", __func__, "mnt", strerror(errno));
            return -2;
        }
    }

	argv += optind;
	argc -= optind;
    execvp(*argv, argv);      // Execute a command in namspace
    errExit("execvp");
}