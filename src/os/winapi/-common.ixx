module;
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
export module os.winapi:common;

#ifdef _WIN32
import <system_error>;
import <string>;

namespace winapi
{
	export using handle_t = HANDLE;
	export using dword_t = DWORD;
	export using secutity_attr_t = SECURITY_ATTRIBUTES;

	export using ::CloseHandle;
	export using ::GetLastError;
	export using ::CreateFileA;
	export using ::WriteFile;
	export using ::ReadFile;
	export using ::CreatePipe;
	export using ::CreateNamedPipeA;
	export using ::PeekNamedPipe;

	export enum class pipe_open_mode: dword_t
	{
		duplex = PIPE_ACCESS_DUPLEX,
		downlink = PIPE_ACCESS_OUTBOUND,
		uplink = PIPE_ACCESS_INBOUND
	};
	export namespace pipe_open_parameters
	{
		constexpr inline dword_t first_instance = FILE_FLAG_FIRST_PIPE_INSTANCE;
		constexpr inline dword_t overlapped = FILE_FLAG_OVERLAPPED;
	};
	export namespace pipe_modes
	{
		constexpr inline dword_t write_byte = PIPE_TYPE_BYTE;
		constexpr inline dword_t write_msg = PIPE_TYPE_MESSAGE;
		constexpr inline dword_t read_byte = PIPE_READMODE_BYTE;
		constexpr inline dword_t read_msg = PIPE_READMODE_MESSAGE;
		constexpr inline dword_t wait = PIPE_WAIT;
		constexpr inline dword_t nowait = PIPE_NOWAIT;
		constexpr inline dword_t accept_remotes = PIPE_ACCEPT_REMOTE_CLIENTS;
		constexpr inline dword_t reject_remotes = PIPE_REJECT_REMOTE_CLIENTS;
	};
	export constexpr inline dword_t unlimited_pipe_instances = PIPE_UNLIMITED_INSTANCES;
	export namespace file_access
	{
		constexpr inline dword_t execute = GENERIC_EXECUTE;
		constexpr inline dword_t read = GENERIC_READ;
		constexpr inline dword_t write = GENERIC_WRITE;
	};
	export namespace file_open_create
	{
		constexpr inline dword_t always_new = CREATE_ALWAYS;
		constexpr inline dword_t only_new = CREATE_NEW;
		constexpr inline dword_t always_open = OPEN_ALWAYS;
		constexpr inline dword_t only_existing = OPEN_EXISTING;
		constexpr inline dword_t truncate_existing = TRUNCATE_EXISTING;
	};
	export namespace file_attributes
	{
		constexpr inline dword_t overlapped = FILE_FLAG_OVERLAPPED;
	};

	export std::system_error last_exception(const char* msg)
	{
		return {static_cast< int >(GetLastError()), std::system_category(), msg};
	}
	export std::system_error last_exception(std::string msg)
	{
		return {static_cast< int >(GetLastError()), std::system_category(), msg};
	}
}
#endif
