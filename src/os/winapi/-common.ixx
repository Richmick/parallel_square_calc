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
	export using process_info_t = PROCESS_INFORMATION;
	export using startup_info_a_t = STARTUPINFOA;

	export using ::CloseHandle;
	export using ::SetHandleInformation;
	export using ::GetStdHandle;
	export using ::GetLastError;
	export using ::CreateFileA;
	export using ::WriteFile;
	export using ::ReadFile;
	export using ::FlushFileBuffers;
	export using ::CreatePipe;
	export using ::CreateNamedPipeA;
	export using ::PeekNamedPipe;
	export using ::CreateProcessA;
	export using ::TerminateProcess;
	export using ::GetExitCodeProcess;
	export using ::SetConsoleCtrlHandler;
	export using ::SetConsoleCtrlHandler;
	export using ::GenerateConsoleCtrlEvent;
	export using ::WaitForSingleObject;
	export using ::WaitForMultipleObjects;

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
	export constexpr inline dword_t still_active_code = STILL_ACTIVE;
	export constexpr inline dword_t inherit_flag = HANDLE_FLAG_INHERIT;
	export constexpr inline dword_t close_protection_flag = HANDLE_FLAG_PROTECT_FROM_CLOSE;
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
	export namespace process_startup
	{
		constexpr inline dword_t new_console = CREATE_NEW_CONSOLE;
		constexpr inline dword_t new_process_group = CREATE_NEW_PROCESS_GROUP;
		constexpr inline dword_t no_window = CREATE_NO_WINDOW;
		constexpr inline dword_t suspended = CREATE_SUSPENDED;
		constexpr inline dword_t detached = DETACHED_PROCESS;

		constexpr inline dword_t use_std_handles = STARTF_USESTDHANDLES;
	};
	export namespace wait_result
	{
		constexpr inline dword_t failed = WAIT_FAILED;
		constexpr inline dword_t success_for_first = WAIT_OBJECT_0;
		constexpr inline dword_t timeout = WAIT_TIMEOUT;
	};
	export enum class control_signal
	{
		sigstop = CTRL_C_EVENT,
		sigint = CTRL_BREAK_EVENT,
		sigclose = CTRL_CLOSE_EVENT,
		siglogoff = CTRL_LOGOFF_EVENT,
		sigshutdown = CTRL_SHUTDOWN_EVENT
	};

	export std::system_error last_exception(const char* msg)
	{
		return {static_cast< int >(GetLastError()), std::system_category(), msg};
	}
	export std::system_error last_exception(std::string msg)
	{
		return {static_cast< int >(GetLastError()), std::system_category(), msg};
	}

	export void disable_inheritance(handle_t handle)
	{
		if (!SetHandleInformation(handle, inherit_flag, 0))
		{
			throw last_exception("failed to disable inheritance for handle");
		}
	}

	export enum class IO: dword_t
	{
		in = STD_INPUT_HANDLE,
		out = STD_OUTPUT_HANDLE,
		err = STD_ERROR_HANDLE
	};
}

#endif
