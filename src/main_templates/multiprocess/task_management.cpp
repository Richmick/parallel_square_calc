module main_templates.multiprocess;

void mains::multiprocess::cmd_controller::add_task()
{
	std::string proc_name, task_name, composition_name;
	std::size_t nthreads;
	std::uint64_t shots;
	std::cin >> proc_name >> task_name >> composition_name >> nthreads >> shots;
	std::size_t proc_id = pids_.at(proc_name);
	daemon_.send(proc_id, "<--open-task--> 0 " + std::to_string(nthreads) + " " + std::to_string(shots)
				+ " 1 circle 0 1 0 0\n");
	log().info("created task #{} on process #{}", 0, proc_id);
}
