#!/usr/bin/env python

import httplib, subprocess, shlex, os, sys, time


def expect_status(*expected):
	def wrapper(fun):
		def inside(self, *args, **kargs):
			(response, body) = fun(self, *args, **kargs)
			if not response.status in expected:
				print "\tFAILURE: expected %s, got %d: %s" % (
						str(expected), response.status, body)
			else:
				print "\tSUCCESS"
			return (response, body)

		return inside
	return wrapper


def expect_body(expected):
	def wrapper(fun):
		def inside(self, *args, **kargs):
			(response, body) = fun(self, *args, **kargs)
			if body != expected:
				print "\tFAILURE: expected %s, got %d: %s" % (
						str(expected), response.status, body)
			else:
				print "\tSUCCESS"
			return (response, body)

		return inside
	return wrapper

def with_server(name):
	def wrapper(fun):
		def inside(self, *args, **kargs):

			oldpwd = os.getcwd()
			os.chdir(os.path.dirname(__file__)) # pushd
			ret = (None, None)

			p = subprocess.Popen("./%s" % name) # start server
			try:
				time.sleep(1)
				ret = fun(self, *args, **kargs)     # run test
			except Exception as e:
				print e
			p.kill()                            # stop server
			p.wait()
			
			os.chdir(oldpwd)                    # popd
			return ret

		return inside
	return wrapper


class StutterTest:
	
	def __init__(self):
		pass

	def request(self, method, uri, body="", headers={}):
		cx = httplib.HTTPConnection("localhost", 8888)
		cx.request(method, uri, body, headers)
		r = cx.getresponse()
		return (r, r.read())

	@expect_body("hello\r\n")
	@expect_status(200)
	@with_server("helloworld")
	def test_helloworld(self):
		return self.request("GET", "/hello")

	@expect_status(403)
	@with_server("helloworld")
	def test_short_dispatcher(self):
		return self.request("GET", "/hell") # shorter shouldn't match

	@expect_body("hello\r\n")
	@expect_status(200)
	@with_server("helloworld")
	def test_long_dispatcher(self):
		return self.request("GET", "/helloooo") # longer should match

	@expect_body("pong")
	@expect_status(200)
	@with_server("proxy")
	def test_proxy(self):
		return self.request("GET", "/proxy") # proxying onto itself

	@expect_body("pongpong")
	@expect_status(200)
	@with_server("proxy")
	def test_proxy(self):
		return self.request("GET", "/double") # two proxy calls one after the other

t = StutterTest()
for m in dir(t):
	if m.startswith("test_"):
		print m
		getattr(t, m)()
