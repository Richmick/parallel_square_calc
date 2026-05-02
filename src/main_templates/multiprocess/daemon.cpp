module main_templates.multiprocess;

mains::multiprocess::io_daemon::io_daemon(cmd_controller& ctrl):
	controller_(ctrl),
	thread_([this](std::stop_token token){operator()(std::move(token));}),
	log_(ctrl.settings_.log)
{
	log_.set_name("io-daemon");
}
mains::multiprocess::io_daemon::~io_daemon()
{
	thread_.request_stop();
	thread_.join();
}
void mains::multiprocess::io_daemon::send(std::size_t proc_id, std::string msg)
{
	std::unique_lock lock{*this};
	io_buffer& cache = buffers_.try_emplace(proc_id).first->second;
	cache.pending += std::move(msg);
}
std::size_t mains::multiprocess::io_daemon::ready()
{
	return results_.size();
}
std::pair< std::size_t, mains::multiprocess::task_result > mains::multiprocess::io_daemon::top()
{
	return results_.back();
}
void mains::multiprocess::io_daemon::pop()
{
	results_.pop_back();
}

void mains::multiprocess::io_daemon::operator()(std::stop_token stop)
{
	log_.debug("io daemon started");
	while (!stop.stop_requested())
	{
		std::this_thread::sleep_for(sleep_time_);
		std::unique_lock lock{*this};
		for (auto& [id, proc]: controller_.processes_)
		{
			if (proc.joinable())
			{
				io_buffer& cache = buffers_.try_emplace(id).first->second;
				try
				{
					if (!cache.pending.empty())
					{
						std::size_t written = proc.get_io(os::IO::in)
								.write_nonblock(cache.pending.c_str(), cache.pending.size());
						if (written != 0)
						{
							cache.pending.erase(0, written);
						}
					}
					constexpr std::size_t buf_size = 256;
					char buf[buf_size];
					std::size_t read = proc.get_io(os::IO::out).read_nonblock(buf, buf_size);
					if (read == 0)
					{
						continue;
					}
					for (std::size_t i = 0; i < read; i++)
					{
						if (cache.catched.empty() && (buf[i] != '<'))
						{
							continue;
						}
						if (buf[i] != '>')
						{
							cache.catched += buf[i];
							continue;
						}
						std::pair< std::size_t, task_result > res;
						std::istringstream msg{cache.catched};
						msg.ignore(1);
						msg >> res.first >> res.second.square;
						if (!msg)
						{
							msg.clear(msg.rdstate() & ~std::ios::failbit);
							std::string fatal;
							msg >> fatal;
							if (fatal != "fatal")
							{
								log_.fatal("failed to parse message: \"{}>\"", cache.catched);
								cache.catched.clear();
								continue;
							}
							res.second.fatal = true;
						}
						res.second.ready = true;
						results_.push_back(res);
						cache.catched.clear();
						notify_all();
						if (!res.second.fatal)
						{
							log_.info("process #{} finished task #{} with result {}", id, res.first, res.second.square);
						}
						else
						{
							log_.info("process #{} finished task #{} because of fatal error", id, res.first);
						}
					}
				}
				catch (const std::system_error&)
				{
					if (!proc.alive())
					{
						log_.info("process #{} is not alive", id);
					}
				}
			}
		}
	}
	log_.debug("io daemon finished");
}
