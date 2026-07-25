export module concurrent.type_suppliers;

import std;

namespace concurrent
{
	export struct std_types
	{
		using mutex = std::mutex;
		using thread = std::thread;
		using condition_variable = std::condition_variable;
	};
}
