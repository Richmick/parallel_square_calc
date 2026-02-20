#include <boost/test/unit_test.hpp>

#ifdef _WIN32
	#define application ".\\..\\..\\bin\\psc_dispatch.exe"
	#define new_line "\r\n"
#endif

import os;
import timer;

using namespace std::chrono_literals;

BOOST_AUTO_TEST_SUITE(PSC_APP_singleprocess_light)

BOOST_AUTO_TEST_CASE(no_args_test)
{
	os::process pgm;
	BOOST_TEST(!pgm.joinable());
	pgm.start(application, application " -s --");
	BOOST_TEST(pgm.join(std::chrono::milliseconds{200}));
	BOOST_TEST(pgm.return_code() == 1);
}
BOOST_AUTO_TEST_CASE(no_stdin_test)
{
	os::process pgm;
	pgm.start(application, application " -s -- 100000 0");
	BOOST_TEST(pgm.join(std::chrono::milliseconds{200}));
	BOOST_TEST(pgm.return_code() == 2);
}
BOOST_AUTO_TEST_CASE(interrupt_test)
{
	os::process pgm;
	pgm.set_io(os::IO::in, os::pipe{100});
	pgm.start(application, application " -s -- 100000 0");
	BOOST_TEST(!pgm.join(std::chrono::milliseconds{50}));
	pgm.sigint();
	if (!pgm.join(std::chrono::milliseconds{200}))
	{
		BOOST_TEST(false, "failed to interrupt");
		pgm.kill();
	}
	BOOST_TEST(pgm.return_code() != os::still_active_code);
}
BOOST_AUTO_TEST_CASE(full_test)
{
	os::process pgm;
	pgm.set_io(os::IO::in, os::pipe{100});
	pgm.set_io(os::IO::out, os::pipe{100});
	pgm.set_io(os::IO::err, os::pipe::get_stdio_pipe(os::IO::err));
	pgm.start(application, application " -s -- 100000 0");
	BOOST_TEST(!pgm.join(100ms));
	BOOST_TEST(pgm.return_code() == os::still_active_code);

	if (!pgm.get_io(os::IO::in).write("1 4", 100ms))
	{
		BOOST_TEST(false);
		pgm.sigint();
		BOOST_TEST(pgm.join(100ms));
		return;
	}
	BOOST_TEST(pgm.join(500ms));

	const char expected[] = "3.14148" new_line;
	std::string out(sizeof(expected) - 1 + 10, '\0');
	BOOST_TEST(pgm.get_io(os::IO::out).read_nonblock(out.data(), out.size()) == sizeof(expected) - 1);
	out.resize(out.find_last_not_of('\0') + 1);
	BOOST_TEST(out == expected);
	BOOST_TEST(pgm.return_code() == 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PSC_APP_singleprocess_heavy)

BOOST_AUTO_TEST_CASE(no_stdin_test)
{
	os::process pgm;
	pgm.start(application, application " -s -- -Sa --seed=0");
	BOOST_TEST(pgm.join(std::chrono::milliseconds{200}));
	BOOST_TEST(pgm.return_code() == 0);
}
BOOST_AUTO_TEST_CASE(interrupt_test)
{
	os::process pgm;
	pgm.set_io(os::IO::in, os::pipe{100});
	os::disable_inheritance(pgm.get_io(os::IO::in).native_write());
	pgm.start(application, application " -s -- -Sa --seed=0");
	BOOST_TEST(!pgm.join(std::chrono::milliseconds{50}));
	pgm.get_io(os::IO::in).release_write();
	pgm.sigint();
	if (!pgm.join(std::chrono::milliseconds{200}))
	{
		BOOST_TEST(false, "failed to interrupt");
		pgm.kill();
	}
	BOOST_TEST(pgm.return_code() != os::still_active_code);
}
BOOST_AUTO_TEST_CASE(full_test)
{
	os::process pgm;
	pgm.set_io(os::IO::in, os::pipe{100});
	pgm.set_io(os::IO::out, os::pipe{100});
	pgm.set_io(os::IO::err, os::pipe::get_stdio_pipe(os::IO::err));
	os::disable_inheritance(pgm.get_io(os::IO::in).native_write());
	os::disable_inheritance(pgm.get_io(os::IO::out).native_read());
	pgm.start(application, application " -s -- -Sa --seed=0");
	BOOST_TEST(!pgm.join(100ms));
	BOOST_TEST(pgm.return_code() == os::still_active_code);

	if (!pgm.get_io(os::IO::in).write("<--open-task--> 0 4 100000 2 "
					"circle 0 1 0 0 rect 1.5707963267 -0.5 -0.5 1.5 0.5\n", 100ms))
	{
		BOOST_TEST(false);
		pgm.sigint();
		BOOST_TEST(pgm.join(100ms));
		return;
	}
	pgm.get_io(os::IO::in).write_nonblock("\0\0\0\0", 4);
	pgm.get_io(os::IO::in).release_write();
	BOOST_TEST(pgm.join(500ms));

	const char expected[] = "! 0 3.57452 !" new_line;
	std::string out(sizeof(expected) - 1 + 10, '\0');
	BOOST_TEST(pgm.get_io(os::IO::out).read_nonblock(out.data(), out.size()) == sizeof(expected) - 1);
	out.resize(out.find_last_not_of('\0') + 1);
	BOOST_TEST(out == expected);
	BOOST_TEST(pgm.return_code() == 0);
}

BOOST_AUTO_TEST_SUITE_END()
