export module os.winapi:pipe;

import std;
import timer;
import :common;
import :unique_handle;

namespace winapi
{
	export struct pipe_create_info
	{
		bool async = false;
		std::size_t max_instances = unlimited_pipe_instances;
		std::size_t read_buf_size = 512;
		std::size_t write_buf_size = 512;
		std::chrono::milliseconds default_timeout{0}; // 0 = system-default (50ms)
	};
	export struct open_tag_t
	{};
	export constexpr inline open_tag_t open_tag = {};

	/// Named pipe and anonymus pipe
	export class pipe
	{
	public:
		pipe() = default;
		pipe(const pipe& rhs) = delete;
		pipe(pipe&& rhs) = default;
		/// Create anonymus pipe (produce both read and write side handles)
		explicit pipe(dword_t size, bool inherit = true)
		{
			secutity_attr_t attr = {
						.nLength = sizeof(secutity_attr_t),
						.lpSecurityDescriptor = nullptr,
						.bInheritHandle = inherit
					};
			if (!CreatePipe(&read_, &write_, &attr, size))
			{
				throw last_exception("failed to create pipe");
			}
		}
		/// Open existing named pipe (as client)
		pipe(open_tag_t, std::string name, pipe_open_mode mode, bool async = false)
		{
			std::string real_name = "\\\\.\\pipe\\LOCAL\\" + name;
			dword_t access = 0;
			switch (mode)
			{
			case pipe_open_mode::duplex:
				access = file_access::write | file_access::read;
				break;
			case pipe_open_mode::uplink:
				access = file_access::write;
				break;
			case pipe_open_mode::downlink:
				access = file_access::read;
				break;
			default:
				throw std::invalid_argument("unknown pipe open mode");
			}
			dword_t attr = (async ? file_attributes::overlapped : 0);
			unique_handle created(CreateFileA(real_name.c_str(), access, 0, nullptr,
							file_open_create::only_existing, attr, nullptr));
			switch (mode)
			{
			case pipe_open_mode::duplex:
				biside_ = std::move(created);
				break;
			case pipe_open_mode::uplink:
				write_ = std::move(created);
				break;
			case pipe_open_mode::downlink:
				read_ = std::move(created);
				break;
			}
		}
		/// Create server-side named pipe
		pipe(std::string name, pipe_open_mode mode, pipe_create_info info, bool inherit = true)
		{
			std::string real_name = "\\\\.\\pipe\\LOCAL\\" + name;
			dword_t open_mode = static_cast< dword_t >(mode) | pipe_open_parameters::first_instance;
			if (info.async) open_mode |= pipe_open_parameters::overlapped;
			dword_t work_mode = pipe_modes::reject_remotes | pipe_modes::nowait
					| pipe_modes::read_byte | pipe_modes::write_byte;
			secutity_attr_t attr = {
						.nLength = sizeof(secutity_attr_t),
						.lpSecurityDescriptor = nullptr,
						.bInheritHandle = inherit
					};
			unique_handle created(CreateNamedPipeA(real_name.c_str(), open_mode, work_mode,
						static_cast< dword_t >(info.max_instances),
						static_cast< dword_t >(info.write_buf_size),
						static_cast< dword_t >(info.read_buf_size),
						static_cast< dword_t >(info.default_timeout.count()), &attr));
			switch (mode)
			{
			case pipe_open_mode::duplex:
				biside_ = std::move(created);
				break;
			case pipe_open_mode::uplink:
				read_ = std::move(created);
				break;
			case pipe_open_mode::downlink:
				write_ = std::move(created);
				break;
			default:
				throw std::invalid_argument("unknown pipe open mode");
			}
		}
		explicit pipe(handle_t nonown_read, handle_t nonown_write) noexcept:
			read_nonown_(nonown_read),
			write_nonown_(nonown_write)
		{}
		pipe(unique_handle read, unique_handle write) noexcept:
			read_(std::move(read)),
			write_(std::move(write))
		{}
		pipe(std::nullptr_t, unique_handle write) noexcept:
			read_(nullptr),
			write_(std::move(write))
		{}
		pipe(unique_handle read, std::nullptr_t) noexcept:
			read_(std::move(read)),
			write_(nullptr)
		{}
		explicit pipe(unique_handle biside) noexcept:
			biside_(std::move(biside))
		{}
		~pipe() = default;

		pipe& operator=(const pipe& rhs) = delete;
		pipe& operator=(pipe&& rhs) = default;

		bool can_read() const noexcept
		{
			return read_ || biside_ || read_nonown_;
		}
		bool can_write() const noexcept
		{
			return write_ || biside_ || write_nonown_;
		}
		operator bool() const noexcept
		{
			return read_ || write_ || biside_ || read_nonown_ || write_nonown_;
		}
		bool operator!() const noexcept
		{
			return !operator bool();
		}

		unique_handle release_read() noexcept
		{
			return std::move(read_);
		}
		unique_handle release_write() noexcept
		{
			return std::move(write_);
		}
		unique_handle release_biside() noexcept
		{
			return std::move(biside_);
		}
		void close()
		{
			biside_.close();
			read_.close();
			write_.close();
			write_nonown_ = nullptr;
			read_nonown_ = nullptr;
		}

		std::size_t write_nonblock(const char* msg, std::size_t len)
		{
			dword_t written = 0;
			if (!WriteFile(native_write(), msg, static_cast< dword_t >(len), &written, nullptr))
			{
				throw last_exception("failed to write to pipe");
			}
			return written;
		}
		bool write(std::span< const char > msg,
					std::chrono::milliseconds timeout = std::chrono::milliseconds{(~1ULL) >> 1},
					std::size_t* written = nullptr)
		{
			std::size_t i = 0;
			chrono::timer timer;
			for (; (i < msg.size()) && (timer.time_since_epoch() <= timeout);
						i += write_nonblock(msg.data() + i, msg.size() - i))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{5});
			}
			if (written != nullptr)
			{
				*written = i;
			}
			return i == msg.size();
		}
		std::size_t available()
		{
			dword_t result = 0;
			if (!PeekNamedPipe(native_read(), nullptr, 0, nullptr, &result, nullptr))
			{
				throw last_exception("failed to read from file");
			}
			return result;
		}
		std::size_t read_nonblock(char* buf, std::size_t len)
		{
			std::size_t avail = available();
			if (avail < len)
			{
				len = avail;
			}
			if (len > 0)
			{
				dword_t result = 0;
				ReadFile(native_read(), buf, static_cast< dword_t >(len), &result, nullptr);
				return result;
			}
			return 0;
		}

		static handle_t get_stdio(IO stream)
		{
			return GetStdHandle(static_cast< dword_t >(stream));
		}
		static pipe get_stdio_pipe(IO stream)
		{
			if (stream == IO::in)
			{
				return pipe{get_stdio(stream), nullptr};
			}
			return pipe{nullptr, get_stdio(stream)};
		}

		handle_t native_read() noexcept
		{
			if (read_) return read_.get();
			if (biside_) return biside_.get();
			if (read_nonown_) return read_nonown_;
			return nullptr;
		}
		handle_t native_write() noexcept
		{
			if (write_) return write_.get();
			if (biside_) return biside_.get();
			if (write_nonown_) return write_nonown_;
			return nullptr;
		}

	private:
		unique_handle read_, write_, biside_;
		handle_t read_nonown_ = nullptr;
		handle_t write_nonown_ = nullptr;
	};
}
