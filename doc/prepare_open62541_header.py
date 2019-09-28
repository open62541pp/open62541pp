#!/usr/bin/env python

import sys
import os
import re

# Remove restructured text documentation from open62541 header files
# (all text in /** */ comments is restructured text)

if len(sys.argv) < 1:
    print("Usage: python prepare_open62541_header.py input.c/h")
    exit(0)

remove_keyword = [" UA_EXPORT", " UA_INLINE", " UA_FUNC_ATTR_WARN_UNUSED_RESULT",
                  " UA_FUNC_ATTR_MALLOC", " UA_RESTRICT "]

def recursive_glob(rootdir='.', suffix=''):
    return [os.path.join(looproot, filename)
            for looproot, _, filenames in os.walk(rootdir)
            for filename in filenames if filename.endswith(suffix)]

for filename in recursive_glob(rootdir=sys.argv[1], suffix='.h'):
    with open(filename, 'r') as f:
        content = f.read()

        # remove rst comment blocks
        content = re.sub('/\*\*[\s\S]+?\*/$', '', content, flags = re.M) # /** ... */

        # transform /* ... */ comments to doxygen style /** ... */
        content = re.sub('^/\* ', '/** ', content, flags = re.M)

        for keyword in remove_keyword:
            content = content.replace(keyword, '')
    
    with open(filename, 'w') as f:
        f.write(content)