export module os.winapi:process;

import <string>;
import <chrono>;
import <span>;
import <exception>;

import :common;
import :pipe;
import :unique_handle;

namespace winapi
{
	/// Process wrapper
	/**
	  * Requires to setup interprocess communication before calling start()
	  * By default destructor calls close()
	  */
	export class process
	{
	public:
		process() = default;
		~process();

		/// Do not add path as first argv
		void start(std::string path, std::string arguments);
		void set_io(IO stream, pipe pipe);
		pipe& get_io(IO stream);

		void sigint();
		void kill();

		bool joinable() const noexcept;
		void detach();
		bool join(std::chrono::milliseconds timeout = std::chrono::milliseconds{-1});
		bool alive(); ///< effectively join(0ms)

		int return_code() const noexcept; ///< result after join
		int id() const noexcept;

	private:
		unique_handle handle_;
		pipe stdin_, stdout_, stderr_;
		int result_code_;
		int id_;
	};
}

winapi::process::~process()
{
	if (joinable())
	{
		std::terminate();
	}
}
void winapi::process::start(std::string path, std::string arguments)
{
	result_code_ = still_active_code;
	process_info_t proc_info{};
	startup_info_a_t startup_info = {
				.cb = sizeof(startup_info),
				.dwFlags = process_startup::use_std_handles,
				.hStdInput = stdin_.native_read(),
				.hStdOutput = stdout_.native_write(),
				.hStdError = stderr_.native_write()
			};
	if (!CreateProcessA(path.data(),
					arguments.data(),
					nullptr, // process handle will not be inherited
					nullptr, // thread handle will not be inherited
					true, // marked as inherited handles will be inherited
					process_startup::new_process_group,
					nullptr, // no new environment
					nullptr, // use parent directory
					&startup_info,
					&proc_info
				))
	{
		throw last_exception("failed to create process");
	}
	id_ = static_cast< int >(proc_info.dwProcessId);
	handle_ = unique_handle{proc_info.hProcess};
	unique_handle{proc_info.hThread}; // no need
	stdin_.release_read();
	stdout_.release_write();
	stderr_.release_write();
}
void winapi::process::set_io(IO stream, pipe pipe)
{
	switch (stream)
	{
	case IO::in:
		stdin_ = std::move(pipe);
		break;
	case IO::out:
		stdout_ = std::move(pipe);
		break;
	case IO::err:
		stderr_ = std::move(pipe);
		break;
	default:
		throw std::invalid_argument("invalid IO enum option");
	}
}
winapi::pipe& winapi::process::get_io(IO stream)
{
	switch (stream)
	{
	case IO::in:
		return stdin_;
	case IO::out:
		return stdout_;
	case IO::err:
		return stderr_;
	default:
		throw std::invalid_argument("invalid IO enum option");
	}
}
void winapi::process::sigint()
{
	if (!GenerateConsoleCtrlEvent(static_cast< dword_t >(control_signal::sigint), id_))
	{
		throw last_exception("failed to send sigint");
	}
}
void winapi::process::kill()
{
	if (!TerminateProcess(handle_.get(), -1))
	{
		throw last_exception("failed to terminate process");
	}
	result_code_ = -1;
	handle_.close();
}

bool winapi::process::joinable() const noexcept
{
	return handle_;
}
void winapi::process::detach()
{
	handle_.close();
	stdin_.close();
	stdout_.close();
	stderr_.close();
}
bool winapi::process::join(std::chrono::milliseconds timeout)
{
	if (!joinable())
	{
		throw std::system_error(std::make_error_code(std::errc::invalid_argument), "joining empty process");
	}
	dword_t res = WaitForSingleObject(handle_.get(), static_cast< dword_t >(timeout.count()));
	if (res == wait_result::failed)
	{
		throw last_exception("failed to wait for process");
	}
	if (res == wait_result::success_for_first)
	{
		dword_t result;
		if (!GetExitCodeProcess(handle_.get(), &result))
		{
			throw last_exception("failed to get exit code (or still active status)");
		}
		result_code_ = static_cast< int >(result);
		handle_.close();
		return true;
	}
	return false;
}
bool winapi::process::alive()
{
	return !join(std::chrono::milliseconds{0});
}
int winapi::process::return_code() const noexcept
{
	return result_code_;
}
int winapi::process::id() const noexcept
{
	return id_;
}
