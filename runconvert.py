#!/usr/bin/env python3
# %% Imports
import os
import glob
import subprocess
import sys

# %% Converter program check
if not os.path.exists('convert.out'):
    ret = subprocess.run('make', stdout = subprocess.PIPE, stderr = subprocess.PIPE, text=True)
    if ret.returncode:
        print(ret.stdout)
        print(ret.stderr)
        sys.exit(ret.returncode)

# %% Argument check
if len(sys.argv) < 2 or len(sys.argv) > 3:
    print('Invocation:\n%s <Input Dir> [Output Dir]\n'%(sys.argv[0]))
    sys.exit(0)

srcdir = sys.argv[1]
# %% Verify source directory
if not os.path.isdir(srcdir):
    raise FileNotFoundError('Directory %s does not exist.'%(srcdir))
# %% Create destination directory
destdir = srcdir.rstrip('/') + '_fits' # default
if len(sys.argv) == 3:
    destdir = sys.argv[2]
if os.path.isfile(destdir):
    raise RuntimeError('Destination %s is a file! Aborting.'%(destdir))
os.makedirs(destdir, exist_ok = True)
# %% Get Catalog File
catfiles = glob.glob('%s/*.cat'%(srcdir))
if len(catfiles) != 1:
    raise RuntimeError('Found %d catalog files. Please check if directory is valid.'%(len(catfiles)))
# %% Read catalog file and convert
ifile = open(catfiles[0], 'r')
for line in ifile:
    fname_ = line.split(' ')[0] # file name is first entry
    fname = '%s/%s'%(srcdir, fname_)
    ret = subprocess.run('./convert.out %s'%(fname), stdout = subprocess.PIPE, stderr = subprocess.PIPE, text=True, shell=True)
    if ret.returncode:
        print('Error converting file %s'%(fname))
        print(ret.stdout)
        print(ret.stderr)
    else:
        try:
            os.rename('./temp.fit', '%s/%s.fit'%(destdir, fname_))
        except Exception:
            if not os.path.exists('./temp.fit'):
                print('Error converting file %s'%(fname))
            else:
                print('Unknown error:\n%s -> %s/%s.fit'%(fname, destdir, fname_))
            continue
        # print('Successfully converted %s/%s.fit'%(destdir, fname_))
ifile.close()
sys.exit(0)
