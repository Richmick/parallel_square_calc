module;
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
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

import <utility>;
import :common;

namespace winapi
{
	class unique_handle
	{
	public:
		unique_handle() noexcept = default;
		unique_handle(const unique_handle& src) = delete;
		unique_handle(unique_handle&& src) noexcept:
			native_(src.release())
		{}
		explicit unique_handle(handle_t handle) noexcept:
			native_(handle)
		{}
		~unique_handle()
		{
			close();
		}

		unique_handle& operator=(const unique_handle& rhs) = delete;
		unique_handle& operator=(unique_handle&& rhs) noexcept
		{
			close();
			native_ = rhs.release();
			return *this;
		}

		void close()
		{
			if (operator bool())
			{
				::CloseHandle(release());
			}
		}
		handle_t get() const noexcept
		{
			return native_;
		}
		handle_t release() noexcept
		{
			return std::exchange(native_, nullptr);
		}
		void swap(unique_handle& rhs) noexcept
		{
			std::swap(native_, rhs.native_);
		}
		
		handle_t* operator&()
		{
			close();
			return &native_;
		}
		operator bool() const noexcept
		{
			return (native_ != nullptr) && (native_ != INVALID_HANDLE_VALUE);
		}
	private:
		handle_t native_ = nullptr;
	};
}

export void std::swap(winapi::unique_handle& lhs, winapi::unique_handle& rhs) noexcept
{
	lhs.swap(rhs);
}

#endif
