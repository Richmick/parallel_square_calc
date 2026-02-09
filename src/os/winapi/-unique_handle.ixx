export module os.winapi:unique_handle;

namespace winapi
{
	export class unique_handle;
}
namespace std
{
	export void swap(winapi::unique_handle& lhs, winapi::unique_handle& rhs) noexcept;
}

#ifdef _WIN32

import :common;

namespace winapi
{
	class unique_handle
	{
	public:
		unique_handle() noexcept = default;
		unique_handle(const unique_handle& src) = delete;
		unique_handle(unique_handle&& src) noexcept;
		explicit unique_handle(handle_t handle) noexcept;
		~unique_handle();

		unique_handle& operator=(const unique_handle& rhs) = delete;
		unique_handle& operator=(unique_handle&& rhs) noexcept;

		void close();
		handle_t get() const noexcept;
		handle_t release() noexcept;
		void swap(unique_handle& rhs) noexcept;

		handle_t* operator&();
		operator bool() const noexcept;
	private:
		handle_t native_ = nullptr;
	};
}

#endif
