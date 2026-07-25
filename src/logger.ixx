export module logger;

import std;

namespace logging
{
	export enum class level
	{
		silent,
		fatal,
		info,
		debug
	};
	export class logger
	{
	public:
		logger(std::ostream& out, int id = -1, logging::level lvl = logging::level::fatal):
			out_(out),
			id_(id),
			lvl_(lvl)
		{}

		int id() const noexcept
		{
			return id_;
		}
		void id(int new_id) noexcept
		{
			id_ = new_id;
		}
		logging::level level() const noexcept
		{
			return lvl_;
		}
		void level(logging::level lvl) noexcept
		{
			lvl_ = lvl;
		}
		void set_name(std::string name)
		{
			name_ = std::move(name);
		}

		template< class... Args >
		void fatal(std::format_string< Args...> format, Args&&... args)
		{
			println(level_name(logging::level::fatal), std::move(format), std::forward< Args >(args)...);
		}
		template< class... Args >
		void info(std::format_string< Args...> format, Args&&... args)
		{
			println(level_name(logging::level::info), std::move(format), std::forward< Args >(args)...);
		}
		template< class... Args >
		void debug(std::format_string< Args...> format, Args&&... args)
		{
			println(level_name(logging::level::debug), std::move(format), std::forward< Args >(args)...);
		}

	private:
		std::ostream& out_;
		int id_;
		logging::level lvl_;
		std::string name_;

		template< class... Args >
		void println(std::pair< logging::level, const char* > lvl,
					std::format_string< Args...> format, Args&&... args)
		{
			if (static_cast< int >(lvl_) < static_cast< int >(lvl.first))
			{
				return;
			}
			std::string msg = std::format(format, std::forward< Args >(args)...);
			if (id_ != -1)
			{
				std::println(out_, "[{}]{{{}#{}, time={}}} {}", lvl.second, name_, id_,
							std::chrono::system_clock::now(), std::move(msg));
			}
			else if (!name_.empty())
			{
				std::println(out_, "[{}]{{{}, time={}}} {}", lvl.second, name_,
							std::chrono::system_clock::now(), std::move(msg));
			}
			else
			{
				std::println(out_, "[{}]{{unknown, time={}}} {}", lvl.second,
							std::chrono::system_clock::now(), std::move(msg));
			}
		}
		consteval std::pair< logging::level, const char* > level_name(logging::level lvl)
		{
			switch (lvl)
			{
			case logging::level::fatal:
				return {logging::level::fatal, "fatal"};
			case logging::level::info:
				return {logging::level::info, "info"};
			case logging::level::debug:
				return {logging::level::debug, "debug"};
			default:
				throw std::invalid_argument("unknown log level");
			}
		}
	};
}
