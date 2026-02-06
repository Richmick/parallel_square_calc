#include <boost/test/unit_test.hpp>

#include <cstdlib>

#ifdef _WIN32
	#define application ".\\..\\..\\bin\\psc_dispatch.exe -s --"
#endif

BOOST_AUTO_TEST_SUITE(PSC_APP_singleprocess)

BOOST_AUTO_TEST_CASE(no_args_test)
{
	BOOST_TEST(std::system(application) == 1);
	BOOST_TEST(std::system(application " -a") == 1);
	BOOST_TEST(std::system("echo 1 8 | " application " 1000000") == 0);
}

BOOST_AUTO_TEST_SUITE_END()
