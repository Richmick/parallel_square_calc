export module main_templates.dispatch;

import <iostream>;
import <print>;
import <vector>;
import <string_view>;
import <span>;
import <stdexcept>;

import flags;
import main_templates.singleprocess;
import main_templates.multiprocess;
import concurrent.type_suppliers;

namespace mains
{
	export int dispatch(int argc, const char*const* argv)
	{
		dispatch::expectation expect;
		expect.add_long_flag("singleprocess", 's');
		expect.add_long_flag("multiprocess", 'm');
		dispatch::flags flags;
		try
		{
			flags.parse({argv, static_cast< std::size_t >(argc)}, expect);
		}
		catch (const std::invalid_argument& e)
		{
			std::println(std::cerr, "invalid arguments format: {}", e.what());
			return 1;
		}
		catch (const std::out_of_range& e)
		{
			std::println(std::cerr, "invalid arguments key-value pairing: {}", e.what());
			return 1;
		}
		if (flags.test('s') && flags.test('m'))
		{
			std::println(std::cerr, "multi- & single- process flags conflicts");
			return 1;
		}

		std::span plain = flags.plain();
		std::vector< const char* > args(plain.size());
		for (std::size_t i = 0; i < plain.size(); i++)
		{
			args[i] = plain[i].data();
		}

		if (flags.test('s'))
		{
			return singleprocess_heavy< concurrent::std_types >(static_cast< int >(args.size()), args.data());
		}
		if (flags.test('m'))
		{
			return multiprocess(static_cast< int >(args.size()), args.data());
		}
		return 3;
	}
}
