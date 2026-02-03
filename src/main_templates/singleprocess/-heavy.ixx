export module main_templates.singleprocess:heavy;

import <iostream>;
import <print>;
import <vector>;
import <future>;
import <span>;

import flags;
import geometry;
import concurrent.thread_pool;

import :common;
import :light;

namespace mains
{
	export struct calculation_task
	{
		std::size_t id;
		geometry::composition_t composed; // concurrent read access
		std::vector< std::future< uint64_t > > results;
	};
	export template< class Typemap >
	int singleprocess_heavy(int argc, const char*const* argv)
	{
		dispatch::expectation expect;
		expect.add_long_flag("slave", 'S');
		expect.add_long_flag("always-online", 'a'); // ignored with no -S
		dispatch::flags flags;
		try
		{
			flags.parse({argv, static_cast< std::size_t >(argc)}, expect);
			if (!flags.test('S'))
			{
				std::span< const std::string_view > plain = flags.plain();
				std::vector< const char* > args(plain.size());
				for (std::size_t i = 0; i < plain.size(); i++)
				{
					args[i] = plain[i].data();
				}
				return singleprocess_light< Typemap >(static_cast< int >(args.size()), args.data());
			}
		}
		catch (const std::exception& e)
		{
			std::println(std::cerr, "{}", e.what());
			return 1;
		}
		bool always_online = flags.test('a');
		return 4;
	}
}
