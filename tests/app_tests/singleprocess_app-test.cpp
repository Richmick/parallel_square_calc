#include <boost/test/unit_test.hpp>

#include <cstdlib>

#ifdef _WIN32
	#define application ".\\..\\..\\bin\\psc_dispatch.exe -s --"
#endif

BOOST_AUTO_TEST_SUITE(PSC_APP_singleprocess)

BOOST_AUTO_TEST_CASE(no_args_test)
{
	BOOST_TEST(std::system(application) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
