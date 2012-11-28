#!/bin/env python
import os

p = os.path

def is_exe(fpath):
	return p.isfile(fpath) and p.exists(fpath) and os.access(fpath, os.X_OK)


errort = []

testdir = p.dirname(os.path.abspath(__file__))
myfile = p.basename(__file__)
for filename in os.listdir(testdir):
	if (filename != myfile and is_exe(p.join(testdir, filename))):
		print("\nRunning: "+filename)
		ret = os.system(p.join(testdir, filename))
		if (ret != 0):
			errort.append(filename)

print "\n"
print("---------------------------------------------------------------------\n")
if (len(errort) == 0):
	print "all tests passed"
else:
	print("These tests may have failed:")
	print(errort)
