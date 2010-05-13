#define USING_CPPUNIT
#define USING_GMOCK

#ifdef USING_CPPUNIT
#		include <fstream>
#		include "cppunit/CompilerOutputter.h"
#		include "cppunit/extensions/TestFactoryRegistry.h"
#		include "cppunit/ui/text/TestRunner.h"
#endif

#ifdef USING_GMOCK
#	include "gmock/gmock.h"
#endif

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

#ifdef USING_GMOCK
	::testing::GTEST_FLAG(throw_on_failure) = true;
	::testing::InitGoogleMock(&argc, argv);
#endif


#ifdef USING_CPPUNIT
#	ifdef USING_XML_OUTPUT
		CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
		CPPUNIT_NS::TextTestRunner runner;
		std::ofstream stream("tests.xml");
		CPPUNIT_NS::XmlOutputter* xml = new CPPUNIT_NS::XmlOutputter(&runner.result(),stream);
		xml->setStyleSheet("report.xsl");
		runner.setOutputter(xml);
		runner.addTest( suite );
		//return runner.run() ? 0 : 1;
		bool result = runner.run();
		return !result;
#	else
		CppUnit::TextUi::TestRunner runner;
		CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
		runner.addTest( registry.makeTest() );
		return !runner.run( "", false );
#	endif
#else
		return 0;
#endif

}

