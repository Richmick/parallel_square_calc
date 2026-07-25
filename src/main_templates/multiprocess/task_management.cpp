module main_templates.multiprocess;

import std;

void mains::multiprocess::cmd_controller::add_task()
{
	std::string proc_name, task_name, composition_name;
	std::size_t nthreads;
	std::uint64_t shots;
	std::cin >> proc_name >> task_name >> composition_name >> nthreads >> shots;
	std::size_t proc_id = pids_.at(proc_name);
	geometry::composition_t& composed = compositions_.at(composition_name);
	if (task_name.starts_with("#"))
	{
		task_name.erase(0, 1);
		task_name += '#';
		task_name += std::to_string(next_task_id_);
		std::println("generated name \"{}\"", task_name);
	}
	if (!task_ids_.try_emplace(task_name, next_task_id_).second)
	{
		throw std::invalid_argument("Entered existing task name");
	}
	tasks_.try_emplace(next_task_id_);
	std::size_t task_id = next_task_id_++;
	std::ostringstream msg;
	msg << "<--open-task--> " << task_id << ' ' << nthreads << ' ' << shots
		<< ' ' << composed.shapes.size();
	for (auto shape: composed.shapes)
	{
		msg << ' ';
		std::visit(name_visitor{msg}, shape.shape);
		msg << ' ' << shape.angle << ' ';
		std::visit(data_visitor{msg}, shape.shape);
	}
	msg << '\n';
	daemon_.send(proc_id, msg.str());
	log().info("created task #{} on process #{}", task_id, proc_id);
}
void mains::multiprocess::cmd_controller::splice_ready()
{
	for (std::size_t i = daemon_.ready(); i-- > 0;)
	{
		auto [id, res] = daemon_.top();
		tasks_.at(id) = res;
		daemon_.pop();
	}
}
void mains::multiprocess::cmd_controller::task_state()
{
	std::string name;
	std::cin >> name;
	const task_result& res = tasks_.at(task_ids_.at(name));
	splice_ready();
	if (!res.ready)
	{
		std::println("<not ready>");
	}
	else
	{
		if (res.fatal)
		{
			std::println("finished due to fatal error");
			return;
		}
		std::println("finished: square = {}", res.square);
	}
}
void mains::multiprocess::cmd_controller::wait_task()
{
	std::string name, postfix;
	long long duration_body;
	std::cin >> name >> duration_body >> postfix;
	std::chrono::milliseconds dur;
	if (postfix == "ms") dur = std::chrono::milliseconds{duration_body};
	else if (postfix == "s") dur = std::chrono::seconds{duration_body};
	else if (postfix == "m") dur = std::chrono::minutes{duration_body};
	else if (postfix == "h") dur = std::chrono::hours{duration_body};
	else throw std::invalid_argument(std::format("unknown time measurement postfix: {}", postfix));
	
	const task_result& res = tasks_.at(task_ids_.at(name));
	std::unique_lock lock{static_cast< std::mutex& >(daemon_)};
	splice_ready();
	if (!res.ready)
	{
		daemon_.wait_for(lock, dur, [this, res] { splice_ready(); return res.ready; });
	}
	if (!res.ready)
	{
		std::println("<not ready>");
	}
	else
	{
		if (res.fatal)
		{
			std::println("finished due to fatal error");
			return;
		}
		std::println("finished: square = {}", res.square);
	}
}
