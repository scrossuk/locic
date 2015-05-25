#!/usr/bin/env python

import ftplib
import os
import sys

server_path = "http://loci-lang.org/travis"

def create_directory(ftp, name):
	try:
		ftp.mkd(name)
	except:
		pass

def upload_path(ftp, path):
	old_cwd = os.getcwd()
	files = os.listdir(path)
	os.chdir(path)
	for f in files:
		full_path = os.path.join(path, f)
		if os.path.isfile(full_path):
			fh = open(f, 'rb')
			sys.stdout.write('Uploading %s...' % full_path)
			ftp.storbinary('STOR %s' % f, fh)
			sys.stdout.write(' done!\n')
			fh.close()
		elif os.path.isdir(full_path):
			create_directory(ftp, f)
			ftp.cwd(f)
			upload_path(ftp, full_path)
			ftp.cwd('..')
	os.chdir(old_cwd)

if len(sys.argv) != 6:
	print "Usage: %s [host] [username] [password] [branch] [path]" % sys.argv[0]
	sys.exit(1)

host = sys.argv[1]
username = sys.argv[2]
password = sys.argv[3]
branch_name = sys.argv[4]
docs_path = sys.argv[5]

ftp = ftplib.FTP(host, username, password)

upload_docs_path = 'docs-%s' % branch_name

if upload_docs_path not in ftp.nlst():
	ftp.mkd(upload_docs_path)

ftp.cwd(upload_docs_path)
upload_path(ftp, os.path.abspath(docs_path))

print "Documentation available at %s" % os.path.join(server_path, upload_docs_path)

ftp.quit()
