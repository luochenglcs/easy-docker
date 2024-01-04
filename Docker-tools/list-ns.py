#!/usr/bin/env python
#coding=utf-8

import os
import re

#format: pid, [namespaces], cmdline
def _get_namespace(pid):
    path = '/proc/{0}/ns/'.format(pid)
    namespaces = []
    for ns in os.listdir(path):
        namespaces.append(os.readlink(path + ns))
    cmdline = open('/proc/{0}/cmdline'.format(pid)).read()
    if not cmdline:
        cmdline = open('/proc/{0}/comm'.format(pid)).read()
    return (pid, namespaces, cmdline)

SBIN_INIT = _get_namespace(1)
OUTPUT = [SBIN_INIT]

for pid in [elt for elt in os.listdir('/proc/') if re.match('\d+', elt)]:
    output = _get_namespace(pid)
    if output[1] != SBIN_INIT[1]:
        OUTPUT.append(output)

for val in OUTPUT:
    print( '{0:>10}    {1}    {2}'.format(
        val[0],
        ' '.join(val[1]),
        ' '.join(val[2].split('\x00'))[:-1]
    ))
