export module os;

export import os.linux;
export import os.winapi;

#ifdef _WIN32
namespace os
{
	export using winapi::open_tag_t;
	export using winapi::open_tag;
	export using winapi::pipe_create_info;
	export using winapi::pipe;
	export using winapi::unique_handle;
}
#endif
