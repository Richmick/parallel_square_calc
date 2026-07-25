module;
#ifdef __linux__
	#include <fcntl.h>
#endif
export module os.linux:unique_handle;

namespace linux
{
	export class unique_handle;
}
namespace std
{
	export void swap(linux::unique_handle& lhs, linux::unique_handle& rhs) noexcept;
}

#ifdef __linux__

import std;
import :common;

namespace linux
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
				fclose(release());
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

export void std::swap(linux::unique_handle& lhs, linux::unique_handle& rhs) noexcept
{
	lhs.swap(rhs);
}

#endif
