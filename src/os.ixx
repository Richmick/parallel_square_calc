export module os;

export import os.linux;
export import os.winapi;

#ifdef _WIN32
namespace os
{
	export using pipe = winapi::pipe;
	export using unique_native_handle = winapi::unique_handle;
}
#endif
