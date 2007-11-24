#!/usr/bin/python

import os

def reformat(directory):
	cmd = "astyle --style=linux --indent=spaces=4 --convert-tabs --unpad=paren"
	# --unpad=paren
	os.system("%s %s/*.cpp" % (cmd, directory))
	os.system("%s %s/*.h" % (cmd, directory))
	os.system("rm -f %s/*.orig" % directory)


reformat("backend")
reformat("widget")
reformat("ladspafx")
reformat("gui")
reformat("mainapp")