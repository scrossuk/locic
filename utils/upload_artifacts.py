#!/usr/bin/env python

import ftplib
import os
import sys

server_path = "http://loci-lang.org/travis"

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
	server_file_path = os.path.join(server_path, dest_filename)
	print "Uploading artifacts file '%s' to server..." % filename
	ftp.storbinary('STOR %s' % os.path.basename(filename), file)
	print "Done! File available at %s" % server_file_path
	file.close()

upload_file(ftp, filename)

ftp.quit()
