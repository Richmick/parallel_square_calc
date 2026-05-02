module main_templates.singleprocess;

#ifdef _WIN32

import os;

namespace mains::singleprocess
{
	static logging::logger* log = nullptr;
	static int __stdcall death_callback(winapi::dword_t signal_id)
	{
		using namespace winapi;
		switch (static_cast< control_signal >(signal_id))
		{
		case control_signal::sigstop:
		case control_signal::sigint:
			std::cin.setstate(std::ios::badbit | std::ios::failbit);
			CloseHandle(pipe::get_stdio(IO::in));
			if (log == nullptr)
			{
				std::println(std::cerr, "recieved stop signal");
			}
			else
			{
				log->fatal("recieved stop signal");
			}
			return true;
		}
		return false;
	}
}
bool mains::singleprocess::register_death_callback(logging::logger* log)
{
	mains::singleprocess::log = log;
	return winapi::SetConsoleCtrlHandler(death_callback, true);
}

#endif
