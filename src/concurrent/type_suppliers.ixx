export module concurrent.type_suppliers;

export import <mutex>;
export import <thread>;
export import <condition_variable>;

namespace concurrent
{
	export struct std_types
	{
		using mutex = std::mutex;
		using thread = std::thread;
		using condition_variable = std::condition_variable;
	};
}
