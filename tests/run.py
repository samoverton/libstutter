#!/usr/bin/env python

import httplib, subprocess, shlex, os, sys


def expect_status(*expected):
	def wrapper(fun):
		def inside(self, *args, **kargs):
			(response, body) = fun(self, *args, **kargs)
			if not response.status in expected:
				print "FAILURE: expected %s, got %d: %s" % (
						str(expected), response.status, body)
			else:
				print "SUCCESS"
			return (response, body)

		return inside
	return wrapper


def expect_body(expected):
	def wrapper(fun):
		def inside(self, *args, **kargs):
			(response, body) = fun(self, *args, **kargs)
			if body != expected:
				print "FAILURE: expected %s, got %d: %s" % (
						str(expected), response.status, body)
			else:
				print "SUCCESS"
			return (response, body)

		return inside
	return wrapper

def with_server(name):
	def wrapper(fun):
		def inside(self, *args, **kargs):

			oldpwd = os.getcwd()
			os.chdir(os.path.dirname(__file__)) # pushd

			p = subprocess.Popen("./%s" % name) # start server
			ret = fun(self, *args, **kargs)     # run test
			p.kill()                            # stop server
			
			os.chdir(oldpwd)                    # popd
			return ret

		return inside
	return wrapper


class StutterTest:
	
	def __init__(self):
		self.cx = httplib.HTTPConnection("localhost", 8888)
		pass

	def request(self, method, uri, body="", headers={}):
		self.cx.request(method, uri, body, headers)
		r = self.cx.getresponse()
		return (r, r.read())

	@with_server("helloworld")
	@expect_status(200)
	@expect_body("hello\r\n")
	def test_helloworld(self):
		return self.request("GET", "/hello")

	@with_server("helloworld")
	@expect_status(403)
	def test_short_dispatcher(self):
		return self.request("GET", "/hell") # shorter shouldn't match

	@with_server("helloworld")
	@expect_status(200)
	@expect_body("hello\r\n")
	def test_long_dispatcher(self):
		return self.request("GET", "/helloooo") # longer should match

t = StutterTest()
for m in dir(t):
	if m.startswith("test_"):
		print m
		getattr(t, m)()
