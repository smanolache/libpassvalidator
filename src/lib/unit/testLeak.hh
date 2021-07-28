#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "pv/intf.hh"

class TestLeak : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TestLeak);
	CPPUNIT_TEST(leak);
	CPPUNIT_TEST_SUITE_END();

	void leak();
public:
	TestLeak();
	~TestLeak();
	void setUp();
	void tearDown();

private:
	void work() const;
	void do_work(pv::App&) const;
};
