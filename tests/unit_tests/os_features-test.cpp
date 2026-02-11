#include <boost/test/unit_test.hpp>

import <chrono>;

import os;
import timer;

BOOST_AUTO_TEST_SUITE(PSC_os)

void test_pipe(os::pipe& rpipe, os::pipe& wpipe)
{
	using namespace std::chrono_literals;
	using namespace std::chrono;

	BOOST_TEST(rpipe.can_read());
	BOOST_TEST(wpipe.can_write());
	BOOST_TEST(rpipe);
	BOOST_TEST(wpipe);

	chrono::timer timer;
	const char msg[] = "allo world";
	constexpr std::size_t len = sizeof(msg);
	std::size_t i = 0;
	for (; (i < len) || (timer.time_since_epoch() > 100ms); i += wpipe.write_nonblock(msg + i, len - i))
	{}
	BOOST_TEST(i == len);
	char buf[len];
	for (i = 0; (i < len) || (timer.time_since_epoch() > 100ms); i += rpipe.read_nonblock(buf + i, len - i))
	{}
	BOOST_TEST(i == len);
	BOOST_CHECK_EQUAL_COLLECTIONS(buf, buf + len, msg, msg + len);
}

BOOST_AUTO_TEST_CASE(anonymus_pipe_test)
{
	os::pipe wpipe(1024);
	BOOST_TEST(wpipe.can_read());
	BOOST_TEST(wpipe.can_write());
	os::pipe rpipe(wpipe.release_read(), nullptr);
	test_pipe(rpipe, wpipe);
}
BOOST_AUTO_TEST_CASE(named_pipe_test)
{
	os::pipe_create_info info = {.max_instances = 2};
	{
		os::pipe server_pipe("PSC_tester", winapi::pipe_open_mode::downlink, info, false);
		BOOST_TEST(!server_pipe.can_read());
		os::pipe client_pipe(os::open_tag, "PSC_tester", winapi::pipe_open_mode::downlink);
		BOOST_TEST(!client_pipe.can_write());
		test_pipe(client_pipe, server_pipe);
	}
	{
		os::pipe server_pipe("PSC_tester", winapi::pipe_open_mode::uplink, info, false);
		BOOST_TEST(!server_pipe.can_write());
		os::pipe client_pipe(os::open_tag, "PSC_tester", winapi::pipe_open_mode::uplink);
		BOOST_TEST(!client_pipe.can_read());
		test_pipe(server_pipe, client_pipe);
	}
	{
		os::pipe server_pipe("PSC_tester", winapi::pipe_open_mode::duplex, info, false);
		os::pipe client_pipe(os::open_tag, "PSC_tester", winapi::pipe_open_mode::duplex);
		test_pipe(client_pipe, server_pipe);
		test_pipe(server_pipe, client_pipe);
	}
}

BOOST_AUTO_TEST_SUITE_END()
