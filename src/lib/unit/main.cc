#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <cppunit/XmlOutputter.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
//#include "parse.hh"

int
main(int argc, char *argv[]) {
//	parse_cmd_line(argc, argv);
	CppUnit::TextUi::TestRunner runner;
	const char *ut_output = getenv("UT_OUTPUT");
	if (0 != ut_output && 0 == strcasecmp("xml", ut_output))
		runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), std::cout));
	runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
	return runner.run() ? 0 : 1;
}
