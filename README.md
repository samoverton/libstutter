# libstutter

libstutter is a C++ library for writing HTTP services, released under the
[2-clause BSD license](COPYING).
It uses [libevent](http://libevent.org/) for its event loop,
[Joyent’s http-parser](https://github.com/joyent/http-parser) to decode HTTP
requests, and [coroutines](http://en.wikipedia.org/wiki/Setcontext) to
switch between clients.

# Aim
The aim of this library is to provide an easy way to write non-blocking HTTP
servers in modern C++ without having to choose between a purely event-driven
model that can be difficult to follow and a purely synchronous approach which
incurs a heavier overhead per connection. libstutter uses coroutines to stop
running threads before they do any blocking IO on a given file descriptor and
resume their execution when the descriptor becomes available.

# Stability
libstutter currently lacks documentation and tests and its API is likely to
change in the future.
The stability and code quality are currently at a “pre-alpha” level.

# TODO

* Add support for multiple handler threads at the event loop level
** + stress test
* Add test for pipelined requests (HTTP/1.1)
* Make error handling better in `Connection`
* Test support for custom headers in Reply object
* Add support for regex routes
    * + test
* Provide helper functions to extract query string parameters
* Test stack overflows in coroutines, find a way to detect issues
* Test backend pool
* Measure performance
    * Benchmarks
    * Avoid copying so many strings
