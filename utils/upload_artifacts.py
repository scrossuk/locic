#!/usr/bin/env python

import ftplib
import os
import sys

if len(sys.argv) != 5:
	print "Usage: %s [host] [username] [password] [file]" % sys.argv[0]
	sys.exit(1)

host = sys.argv[1]
username = sys.argv[2]
password = sys.argv[3]
filename = sys.argv[4]

ftp = ftplib.FTP(host, username, password)

def show_files(ftp):
	ftp.retrlines('LIST')

def upload_file(ftp, filename):
	file = open(filename, 'rb')
	dest_filename = os.path.basename(filename)
	print "Uploading file '%s' to '<server>/%s'..." % (filename, dest_filename)
	ftp.storbinary('STOR %s' % os.path.basename(filename), file)
	print "Done!"
	file.close()

upload_file(ftp, filename)

ftp.quit()
