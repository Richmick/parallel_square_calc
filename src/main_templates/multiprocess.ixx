export module main_templates.multiprocess;

import <iostream>;
import <random>;
import <string_view>;
import <string>;
import <mutex>;
import <condition_variable>;
import <thread>;
import <map>;
import logger;
import os;
import geometry;

namespace mains::multiprocess
{
	struct init_data
	{
		logging::logger log{std::clog};
		std::mt19937_64 engine{0};
		std::string_view executor;
		std::string_view executor_args;
		bool help_only = false;
		bool recursive_executor = true;

		void init(int argc, const char* const* argv);
	};
	struct task_result
	{
		float square;
		bool ready = false, fatal = false;
	};

	class cmd_controller;
	class io_daemon: public std::mutex, public std::condition_variable
	{
	public:
		io_daemon(cmd_controller& ctrl);
		~io_daemon();

		void send(std::size_t proc_id, std::string msg);

		std::size_t ready();
		std::pair < std::size_t, task_result > top();
		void pop();

	private:
		struct io_buffer
		{
			std::string pending;
			std::string catched;
		};
		struct catched_result
		{
			std::size_t id;
			float square;
		};

		cmd_controller& controller_;
		std::chrono::milliseconds sleep_time_{50};
		std::jthread thread_;
		logging::logger log_;
		std::map< std::size_t, io_buffer > buffers_;
		std::vector< std::pair< std::size_t, task_result > > results_;

		void operator()(std::stop_token stop);
	};
	class cmd_controller
	{
	public:
		cmd_controller(init_data data);
		cmd_controller(cmd_controller&&) = delete;
		~cmd_controller();

		void spawn(); ///< Create new process
		void interrupt(); ///< Interrupt certain process (close input & send sigint)
		void terminate(); ///< Terminate certain process without completing tasks
		void add_task(); ///< Create new task on certain process
		void wait_task(); ///< Wait for certain task completion
		void task_state(); ///< Pop all ready task results & check for certain cached
		void add_shape(); ///< Add named shape to map
		void add_composition(); ///< Combine named shapes to single composition
		void print_help(); ///< Print full manual on commands --help -h
		void print_command_names(); ///< Print command names only
		void print_alive(); ///< Print all alive processes (their names)
		void print_shape();
		void print_all_shapes();
		void print_composition();
		void print_compositions_names();
		void calculate_shape_frame();
		void calculate_composition_frame();

		logging::logger& log();

	private:
		friend class io_daemon;

		init_data settings_;
		std::map< std::string, geometry::shape_t > shapes_;
		std::map< std::string, geometry::composition_t > compositions_;
		std::map< std::string, std::size_t > pids_;
		std::map< std::size_t, os::process > processes_;
		std::map< std::string, std::size_t > task_ids_;
		std::map< std::size_t, task_result > tasks_;
		io_daemon daemon_;
		std::size_t next_pid_ = 1, next_task_id_ = 0;

		void splice_ready(); ///< Requires lock on io_daemon
	};
	void print_help();
	struct data_visitor
	{
		std::ostream& out;
		void operator()(geometry::circle_t c);
		void operator()(geometry::rect_t r);
		void operator()(geometry::ellipse_t e);
	};
	struct name_visitor
	{
		std::ostream& out;
		void operator()(const geometry::circle_t&);
		void operator()(const geometry::rect_t&);
		void operator()(const geometry::ellipse_t&);
	};

	export int multiprocess(int argc, const char* const* argv);
}
