export module timer;

export import std;

namespace chrono
{
	export class timer
	{
	public:
		timer() noexcept:
			start_(std::chrono::high_resolution_clock::now())
		{}
		void reset() noexcept
		{
			start_ = std::chrono::high_resolution_clock::now();
		}
		auto time_since_epoch() noexcept
		{
			return std::chrono::high_resolution_clock::now() - start_;
		}
	private:
		std::chrono::high_resolution_clock::time_point start_;
	};
}
