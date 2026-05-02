module main_templates.multiprocess;

import <stdexcept>;
import <print>;
import <iostream>;
import flags;
import main_templates.singleprocess;
import concurrent.type_suppliers;

int mains::multiprocess::multiprocess(int argc, const char*const* argv)
{
	if ((argc > 1) && (std::string_view(argv[1]) == "slave"))
	{
		std::vector< const char* > args(argc - 1);
		args[0] = argv[0];
		for (std::size_t i = 2; i < argc; i++)
		{
			args[i - 1] = argv[i];
		}
		return mains::singleprocess::heavy< concurrent::std_types >(static_cast< int >(args.size()), args.data());
	}
	init_data data;
	try
	{
		data.init(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::println(std::cerr, "{}", e.what());
		return 1;
	}
	if (data.help_only)
	{
		return 0;
	}
	cmd_controller controller(std::move(data));
	std::map< std::string_view, void(cmd_controller::*)() > commands = {
				{"spawn", &cmd_controller::spawn},
				{"send", &cmd_controller::add_task},
				{"state", &cmd_controller::task_state},
				{"wait", &cmd_controller::wait_task},
				{"print-alive", &cmd_controller::print_alive},
				{"help", &cmd_controller::print_help},
				{"cmds", &cmd_controller::print_command_names},
				{"int", &cmd_controller::interrupt},
				{"kill", &cmd_controller::terminate},
				{"create", &cmd_controller::add_shape},
				{"combine", &cmd_controller::add_composition},
				{"shape", &cmd_controller::print_shape},
				{"all-shapes", &cmd_controller::print_all_shapes},
				{"frame", &cmd_controller::calculate_shape_frame},
				{"composition", &cmd_controller::print_composition},
				{"composition-frame", &cmd_controller::calculate_composition_frame},
				{"composition-names", &cmd_controller::print_compositions_names},
			};
	std::string cmd;
	while (std::cin >> cmd)
	{
		controller.log().debug("got command");
		auto pos = commands.find(cmd);
		if (pos == commands.end())
		{
			std::println(std::cerr, "unknown command");
			continue;
		}
		try
		{
			std::cin.exceptions(std::ios_base::failbit);
			(controller.*(pos->second))();
		}
		catch (const std::ios_base::failure&)
		{
			std::println(std::cerr, "failed to parse command (skipping line)");
			std::cin.exceptions(std::ios_base::goodbit);
			std::cin.clear(std::cin.rdstate() & ~std::ios_base::failbit);
			std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
		}
		catch (const std::exception& e)
		{
			std::println(std::cerr, "failed to execute command: {}", e.what());
		}
		std::cin.exceptions(std::ios_base::goodbit);
	}
	return 0;
}
void mains::multiprocess::init_data::init(int argc, const char*const* argv)
{
	dispatch::expectation expect;
	expect.add_long_flag("help", 'h');
	expect.add_long_flag("debug");
	expect.add_long_flag("info");
	expect.add_long_flag("fatal");
	expect.add_long_flag("silent");
	expect.add_key_value("executor");
	expect.add_key_value("executor-args");
	expect.add_key_value("seed");
	dispatch::flags flags({argv, static_cast< std::size_t >(argc)}, expect);
	if (flags.test('h'))
	{
		print_help();
		help_only = true;
		return;
	}
	if (flags.has_key("executor"))
	{
		executor = flags["executor"];
		recursive_executor = false;
	}
	else
	{
		executor = argv[0];
	}
	if (flags.has_key("executor-args"))
	{
		executor_args = flags["executor-args"];
	}
	if (flags.has_key("seed"))
	{
		engine.seed(std::stoull(flags["seed"].data()));
	}
	bool silent = flags.test("silent");
	bool fatal = flags.test("fatal");
	bool info = flags.test("info");
	bool debug = flags.test("debug");
	if (silent + fatal + info + debug > 1)
	{
		throw std::invalid_argument("many log levels were chosen");
	}
	if (fatal) log.level(logging::level::fatal);
	if (info) log.level(logging::level::info);
	if (debug) log.level(logging::level::debug);
	log.set_name("controller");
	log.debug("logger init finished");
}
void mains::multiprocess::print_help()
{
	std::println("NAME:\n"
				"\tpsc - parallel square calculation with Monte-Carlo method (interactive console application)\n"
				"SYNOPSIS:\n"
				"\tpsc slave (-S | --slave) [-a | --always-online] [--debug | --info | --fatal | --silent]\n"
				"\t\t[--seed=<seed>] [--id=<logger-id>] [--start-threads=<nthreads>]\n"
				"\tpsc [--debug | --info | --fatal | --silent] [--executor=<path-to-application>]\n"
				"\t\t[--executor-args=<additional-arguments-for-all-slaves>] [--seed=<seed>]\n"
				"DESCRIPTION:\n"
				"\tThis application collects shapes & their compositions and send it to child processes (slaves).\n"
				"\tSuch processes can be compiled in another binary or this binary can be used instead\n"
				"\t(default-behavior). Slave processes use multithreading for faster calculations.\n"
				"\tBoth usual & slave processes are interactive.\n"
				"OPTIONS:\n"
				"\t-S --slave\n"
				"\t\tPrevent from starting light version\n"
				"\t-a --always-online\n"
				"\t\tDo not use main thread for calculations, e.g. always parse input\n"
				"\t--debug --info --fatal --silent\n"
				"\t\tSet corresponding log level (silent disables logging)\n"
				"\t--seed=<seed>\n"
				"\t\tSet seed for random generators (default = 0)\n"
				"\t--id=<logger-id>\n"
				"\t\tSet logger id (postfix #id for name)\n"
				"\t--start-threads=<nthreads>\n"
				"\t\tSpecify how many threads should be created at start (can be extended in runtime)\n"
				"\t--executor=<path-to-application>\n"
				"\t\tSpecify path to slave executable (no \"slave\" will be passed as first argument by default)\n"
				"\t--executor-args=<additional-arguments-for-all-slaves>\n"
				"\t\tSet additional arguments transmitted to all child processes as first arguments\n"
				"COMMANDS:\n"
				"\tspawn <proc-name>\n"
				"\t\tSpawn child process with recognition name <proc-name> and generated seed\n"
				"\tsend <proc-name> [#]<task-name> <composition-name> <nthreads> <shots>\n"
				"\t\tSend calculation task to child process. If <task-name> starts with '#' then\n"
				"\t\tthis prefix will be removed and postfix \"#<generated-id>\" will be added\n"
				"\tstate <task-name>\n"
				"\t\tRequest for current task state\n"
				"\twait <task-name> <duration-value>(ms|s|m|h)\n"
				"\t\tWait for task completion with duration restriction\n"
				"\tprint-alive\n"
				"\t\tPrint all alive processes with format \"id) name\" \n"
				"\thelp\n"
				"\t\tPrint this message\n"
				"\tcmds\n"
				"\t\tPrint only interactive commands with parameters (with no description)\n"
				"\tint <proc-name>\n"
				"\t\tSend sigint to child process (process will exit after finishing started calculations)\n"
				"\tkill <proc-name>\n"
				"\t\tTerminate process\n"
				"\tcreate <shape-name> circle <radius> <center.x> <center.y>\n"
				"\tcreate <shape-name> rect <p1.x> <p1.y> <p2.x> <p2.y>\n"
				"\tcreate <shape-name> ellipse <h_semiaxis> <v_semiaxis> <center.x> <center.y>\n"
				"\t\tCreate named shape\n"
				"\tcombine <composition-name> (<shape-names> <rotation>)... ;\n"
				"\t\tCreate named composition\n"
				"\tshape <shape-name>\n"
				"\t\tPrint shape data\n"
				"\tall-shapes\n"
				"\t\tPrint all shapes\n"
				"\tframe <shape-name>\n"
				"\t\tCalculate frame rectangle for shape\n"
				"\tcomposition <composition-name>\n"
				"\t\tPrint composed shapes data\n"
				"\tcomposition-frame <composition-name>\n"
				"\t\tCalculate frame rectangle for composition\n"
				"\tcomposition-names\n"
				"\t\tPrints created composition names"
			);
}
void mains::multiprocess::cmd_controller::print_help()
{
	mains::multiprocess::print_help();
}
void mains::multiprocess::cmd_controller::print_command_names()
{
	std::println("* spawn <proc-name>\n"
				"* send <proc-name> [#]<task-name> <composition-name> <nthreads> <shots>\n"
				"* state <task-name>\n"
				"* wait <task-name> <duration-value>(ms|s|m|h)\n"
				"* print-alive\n"
				"* help\n"
				"* cmds\n"
				"* int <proc-name>\n"
				"* kill <proc-name>\n"
				"* create <shape-name> circle <radius> <center.x> <center.y>\n"
				"* create <shape-name> rect <p1.x> <p1.y> <p2.x> <p2.y>\n"
				"* create <shape-name> ellipse <h_semiaxis> <v_semiaxis> <center.x> <center.y>\n"
				"* combine <composition-name> (<shape-names> <rotation>)... ;\n"
				"* shape <shape-name>\n"
				"* all-shapes\n"
				"* frame <shape-name>\n"
				"* composition <composition-name>\n"
				"* composition-frame <composition-name>\n"
				"* composition-names");
}
