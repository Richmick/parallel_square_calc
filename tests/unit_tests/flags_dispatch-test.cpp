#include <boost/test/unit_test.hpp>

import flags;

BOOST_AUTO_TEST_SUITE(PSC_flags_dispatch)

BOOST_AUTO_TEST_CASE(plain_args_test)
{
	const char* const args[] = {"first", "second", "third"};
	dispatch::expectation expect;
	dispatch::flags flags(args);
	BOOST_TEST(!flags.test('f'));
	BOOST_TEST(!flags.test('\0'));
	BOOST_TEST(!flags.test(' '));
	BOOST_TEST(!flags.test("flag"));
	BOOST_CHECK_THROW(flags["key"], std::out_of_range);
	BOOST_CHECK_EQUAL_COLLECTIONS(flags.plain().begin(), flags.plain().end(), args, std::end(args));

	flags.clear();
	flags.parse(args, expect);
	BOOST_TEST(!flags.test('f'));
	BOOST_TEST(!flags.test('\0'));
	BOOST_TEST(!flags.test(' '));
	BOOST_TEST(!flags.test("flag"));
	BOOST_CHECK_THROW(flags["key"], std::out_of_range);
	BOOST_CHECK_EQUAL_COLLECTIONS(flags.plain().begin(), flags.plain().end(), args, std::end(args));
}
BOOST_AUTO_TEST_CASE(unrestricted_test)
{
	const char* const args[] = {"--key1=", "--key2=value2", "third", "-f", "--flag", "-fmt", "--flag",
				"--", "--nokey=novalue", "--noflag", "plain"};
	const char* const plain[] = {"third", "--nokey=novalue", "--noflag", "plain"};
	dispatch::flags flags(args);
	BOOST_TEST(flags.test('f'));
	BOOST_TEST(flags.test('m'));
	BOOST_TEST(flags.test('t'));
	BOOST_TEST(!flags.test("key1"));
	BOOST_TEST(!flags.test("key2"));
	BOOST_TEST(!flags.test("third"));
	BOOST_TEST(flags.test("flag"));
	BOOST_TEST(!flags.test("noflag"));
	BOOST_TEST(flags["key1"] == "");
	BOOST_TEST(flags["key2"] == "value2");
	BOOST_CHECK_THROW(flags["flag"], std::out_of_range);
	BOOST_CHECK_THROW(flags["third"], std::out_of_range);
	BOOST_CHECK_THROW(flags["nokey"], std::out_of_range);
	BOOST_CHECK_EQUAL_COLLECTIONS(flags.plain().begin(), flags.plain().end(), plain, std::end(plain));

	const char* const wrong1[] = {"--="};
	const char* const wrong2[] = {"- "};
	const char* const duplicate1[] = {"--key1=smth"};
	BOOST_CHECK_THROW(flags.parse(wrong1), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(wrong2), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(duplicate1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(restricted_test)
{
	const char* const args[] = {"--key1=", "--key2=value2", "third", "-key3", "value3", "-f", "--flag",
				"-fmt", "--flag", "--", "--nokey=novalue", "--noflag", "plain"};
	const char* const plain[] = {"third", "--nokey=novalue", "--noflag", "plain"};
	dispatch::expectation expect;
	expect.add_monoflag('f');
	expect.add_monoflag('m');
	expect.add_monoflag('t');
	expect.add_long_flag("flag");
	expect.add_key_value("key1");
	expect.add_key_value("key2");
	expect.add_value_after_key("-key3");

	dispatch::flags flags(args, expect);
	BOOST_TEST(flags.test('f'));
	BOOST_TEST(flags.test('m'));
	BOOST_TEST(flags.test('t'));
	BOOST_TEST(!flags.test("key1"));
	BOOST_TEST(!flags.test("key2"));
	BOOST_TEST(!flags.test("-key3"));
	BOOST_TEST(!flags.test("third"));
	BOOST_TEST(flags.test("flag"));
	BOOST_TEST(!flags.test("noflag"));
	BOOST_TEST(flags["key1"] == "");
	BOOST_TEST(flags["key2"] == "value2");
	BOOST_TEST(flags["-key3"] == "value3");
	BOOST_CHECK_THROW(flags["flag"], std::out_of_range);
	BOOST_CHECK_THROW(flags["third"], std::out_of_range);
	BOOST_CHECK_THROW(flags["nokey"], std::out_of_range);
	BOOST_CHECK_EQUAL_COLLECTIONS(flags.plain().begin(), flags.plain().end(), plain, std::end(plain));

	const char* const wrong1[] = {"--="};
	const char* const wrong2[] = {"- "};
	const char* const duplicate1[] = {"--key1=smth"};
	const char* const novalue1[] = {"-key3"};
	const char* const unknown1[] = {"--flag1"};
	const char* const unknown2[] = {"-?"};
	const char* const unknown3[] = {"--key4=smth"};
	BOOST_CHECK_THROW(flags.parse(wrong1, expect), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(wrong2, expect), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(duplicate1, expect), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(novalue1, expect), std::out_of_range);
	BOOST_CHECK_THROW(flags.parse(unknown1, expect), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(unknown2, expect), std::invalid_argument);
	BOOST_CHECK_THROW(flags.parse(unknown3, expect), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
