module main_templates.multiprocess;

import <print>;
import <utility>;

mains::multiprocess::cmd_controller::cmd_controller(init_data data):
	settings_(std::move(data)),
	daemon_(*this)
{}
mains::multiprocess::cmd_controller::~cmd_controller()
{
	for (auto& [id, proc]: processes_)
	{
		if (proc.joinable())
		{
			try
			{
				proc.get_io(os::IO::in).release_write();
				proc.sigint();
				proc.join();
			}
			catch (...)
			{}
		}
	}
}
logging::logger& mains::multiprocess::cmd_controller::log()
{
	return settings_.log;
}

void mains::multiprocess::cmd_controller::spawn()
{
	std::string name;
	std::cin >> name;
	auto ins = pids_.try_emplace(name, ++last_id_);
	if (!ins.second) throw std::invalid_argument("Entered existing name");
	os::process proc;
	proc.set_io(os::IO::in, os::pipe{100});
	proc.set_io(os::IO::out, os::pipe{100});
	proc.set_io(os::IO::err, os::pipe::get_stdio_pipe(os::IO::err));
	os::disable_inheritance(proc.get_io(os::IO::in).native_write());
	os::disable_inheritance(proc.get_io(os::IO::out).native_read());
	std::string args = "\"" + std::string(settings_.executor)
			+ "\"" + (settings_.recursive_executor ? " slave" : "")
			+ " -S --seed=" + std::to_string(settings_.engine())
			+ " --id=" + std::to_string(last_id_)
			+ " " + settings_.executor_args.data();
	log().debug("prepared to start child process");
	proc.start(std::string(settings_.executor), args);
	processes_.try_emplace(ins.first->second, std::move(proc));
	log().info("started child process #{}", last_id_);
}
void mains::multiprocess::cmd_controller::print_alive()
{
	std::size_t i = 0;
	for (const auto& [name, id]: pids_)
	{
		auto proc = processes_.find(id);
		bool t1 = proc == processes_.end();
		bool t2 = proc->second.alive();
		if ((proc == processes_.end()) || !proc->second.alive())
		{
			continue;
		}
		++i;
		std::println("{}) {}", id, name);
	}
	if (i == 0)
	{
		std::println("<no alive processes>");
	}
}
void mains::multiprocess::cmd_controller::interrupt()
{
	std::string name;
	std::cin >> name;
	os::process& proc = processes_.at(pids_.at(name));
	if (proc.joinable())
	{
		proc.get_io(os::IO::in).release_write();
		proc.sigint();
		log().info("send sigint to process \"{}\"", name);
	}
}
void mains::multiprocess::cmd_controller::terminate()
{
	std::string name;
	std::cin >> name;
	os::process& proc = processes_.at(pids_.at(name));
	if (proc.joinable())
	{
		proc.kill();
		log().info("send sigkill to process \"{}\"", name);
	}
}
