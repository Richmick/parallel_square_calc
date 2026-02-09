module;
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
export module os.winapi:common;

#ifdef _WIN32
namespace winapi
{
	export using handle_t = HANDLE;
	export using dword_t = DWORD;

	export using ::CloseHandle;

	export enum class pipe_open_mode: dword_t
	{
		duplex = PIPE_ACCESS_DUPLEX,
		downlink = PIPE_ACCESS_OUTBOUND,
		uplink = PIPE_ACCESS_INBOUND
	};
}
#endif
