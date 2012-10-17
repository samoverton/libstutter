#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

class TestQuitHandler : public BaseHandler {
public:
	virtual void handle(http::Connection &cx, const http::Request &req, http::Reply &reply)
	{
		_exit(0);
	}
};

#endif // TEST_FRAMEWORK_H
