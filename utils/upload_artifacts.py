#!/usr/bin/env python

import artifact
import ftplib
import os
import sys

server_path = "http://loci-lang.org/travis"
num_builds_to_keep = 5

def get_artifact_archives(ftp):
	files = ftp.nlst()
	
	artifact_archives = []
	
	for filename in files:
		artifact_file = artifact.parse_filename(filename)
		if artifact_file is None:
			continue
		artifact_archives.append(artifact_file)
	
	return artifact_archives

def upload_file(ftp, filename):
	file = open(filename, 'rb')
	dest_filename = os.path.basename(filename)
	server_file_path = os.path.join(server_path, dest_filename)
	sys.stdout.write("Uploading artifacts file '%s' to server..." % filename)
	ftp.storbinary('STOR %s' % os.path.basename(filename), file)
	sys.stdout.write(" done! File available at %s\n" % server_file_path)
	file.close()

def get_branch_builds_to_delete(artifact_archives):
	branch_builds = {}
	for artifact_file in artifact_archives:
		if artifact_file.branch not in branch_builds:
			branch_builds[artifact_file.branch] = set()
		branch_builds[artifact_file.branch].add(artifact_file.build_number)
	
	delete_branch_builds = {}
	
	for branch_name, build_numbers in branch_builds.iteritems():
		delete_branch_builds[branch_name] = []
		build_number_list = sorted(build_numbers)
		if len(build_number_list) <= num_builds_to_keep:
			continue
		delete_range = len(build_number_list) - num_builds_to_keep
		for delete_index in range(0, delete_range):
			delete_branch_builds[branch_name].append(build_number_list[delete_index])
	
	return delete_branch_builds

def delete_old_artifacts(ftp):
	artifact_archives = get_artifact_archives(ftp)
	
	delete_branch_builds = get_branch_builds_to_delete(artifact_archives)
	
	for artifact_file in artifact_archives:
		if artifact_file.build_number in delete_branch_builds[artifact_file.branch]:
			sys.stdout.write("Deleting old artifact '%s'..." % artifact_file.get_filename())
			ftp.delete(artifact_file.get_filename())
			sys.stdout.write(" done!\n")

if len(sys.argv) != 5:
	print "Usage: %s [host] [username] [password] [file]" % sys.argv[0]
	sys.exit(1)

host = sys.argv[1]
username = sys.argv[2]
password = sys.argv[3]
filename = sys.argv[4]

ftp = ftplib.FTP(host, username, password)

if artifact.parse_filename(os.path.basename(filename)) is None:
	print "Invalid artifacts file '%s'." % filename
	sys.exit(1)

upload_file(ftp, filename)
delete_old_artifacts(ftp)

ftp.quit()
