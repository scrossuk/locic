#!/usr/bin/env python

import re

artifact_file_pattern = 'locic-artifacts-([0-9]+)\.([0-9]+)-([^-]+)-([^-]+)\.tar\.xz'

class ArtifactFile(object):
	def __init__(self, branch, build_number, job_number, os_name):
		self.branch = branch
		self.build_number = build_number
		self.job_number = job_number
		self.os_name = os_name
	
	def get_filename(self):
		return 'locic-artifacts-%d.%d-%s-%s.tar.xz' % \
		       (self.build_number, self.job_number, self.os_name, self.branch)
	
	def __repr__(self):
		return 'ArtifactFile(branch=\'%s\', build_number=%d, job_number=%s, os_name=\'%s\')' % \
		       (self.branch, self.build_number, self.job_number, self.os_name)

def parse_filename(filename):
	m = re.match(artifact_file_pattern, filename)
	if not m:
		return None
	build_number = int(m.group(1))
	job_number = int(m.group(2))
	os_name = m.group(3)
	branch_name = m.group(4)
	return ArtifactFile(branch_name, build_number, job_number, os_name)
