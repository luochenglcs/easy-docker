#!/usr/bin/env python
#coding=utf-8

## refrence： https://zhuanlan.zhihu.com/p/25576438
import argparse
import ctypes
import os


CLONE_NEWNS =        0x00020000	# /* New mount namespace group */
CLONE_NEWCGROUP =    0x02000000	# /* New cgroup namespace */
CLONE_NEWUTS =       0x04000000	# /* New utsname namespace */
CLONE_NEWIPC =       0x08000000	# /* New ipc namespace */
CLONE_NEWUSER =      0x10000000	# /* New user namespace */
CLONE_NEWPID =       0x20000000	# /* New pid namespace */
CLONE_NEWNET =       0x40000000	# /* New network namespace */



parser = argparse.ArgumentParser()
parser.add_argument('--pid', type = str, help = 'process id')
args = parser.parse_args()
if not args.pid:
    print 'plz input pid..'
    exit(1)


#setns
libc = ctypes.CDLL('libc.so.6')
namespace = [
    ('ipc', CLONE_NEWIPC),
    ('uts', CLONE_NEWUTS),
    ('net', CLONE_NEWNET),
    ('pid', CLONE_NEWPID),
    ('mnt', CLONE_NEWNS),
]
for ns_type, ns_flag in namespace:
    fd = os.open('/proc/{0}/ns/{1}'.format(args.pid, ns_type), os.O_RDONLY)
    ret = libc.setns(fd, ns_flag)
    os.close(fd)
    if ret == -1:
        print 'libc.setns failed'
        exit(1)


#child exec shell
pid = os.fork()
if pid != 0: #father
    os.waitpid(pid, 0)
else: #child
    shell = os.getenv('SHELL')
    os.execl(shell, os.path.basename(shell))