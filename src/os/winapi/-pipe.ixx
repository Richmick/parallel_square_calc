export module os.winapi:pipe;

import <string>;

import :common;
import :unique_handle;

namespace winapi
{
	/// Named pipe and anonymus pipe
	export class pipe
	{
		pipe() = default;
		pipe(const pipe& rhs) = delete;
		pipe(pipe&& rhs);
		pipe(unique_handle read, unique_handle write);
		explicit pipe(dword_t size, bool inherit = true);
		explicit pipe(handle_t nonown);
		explicit pipe(unique_handle biside);
		~pipe();

		pipe& operator=(const pipe& rhs) = delete;
		pipe& operator=(pipe&& rhs);

		void create(std::string name, pipe_open_mode mode, bool inherit = true);
		void open(std::string name, pipe_open_mode mode);

		void set_overlapped(bool new_value) noexcept;
		void set_read_buf_size(std::size_t new_value) noexcept;
		void set_write_buf_size(std::size_t new_value) noexcept;

		bool can_read() const noexcept;
		bool can_write() const noexcept;

		std::size_t write_nonblock(const char* msg, std::size_t len);
		std::size_t read_nonblock(char* buf, std::size_t len);

	private:
		unique_handle read_, write_, biside_;
		handle_t read_nonown_ = nullptr;

		bool overlapped_ = false;
		std::size_t read_buf_size_ = 512;
		std::size_t write_buf_size_ = 512;
	};
}
