export module main_templates.singleprocess:heavy;

import std;

import logger;
import geometry;
import concurrent.thread_pool;

import :common;
import :light;

namespace mains::singleprocess
{
	export template< class Typemap >
	int heavy(int argc, const char*const* argv);

	struct calculation_task
	{
		std::ostream& out;
		std::size_t id;
		std::size_t nthreads;
		std::uint64_t shots;
		geometry::rect_t frame;
		geometry::composition_t composed;
		std::vector< std::pair< std::size_t, std::future< std::uint64_t > > > results;
		bool finished = false;

		void success(std::uint64_t hits)
		{
			std::println(out, "< {} {} >", id, shots_to_square(frame, shots, hits));
			finished = true;
		}
		void fatal()
		{
			std::println(out, "< {} fatal >", id);
			finished = true;
		}
	};
	template< class Pool >
	struct autosubmit_monothread
	{
		Pool& pool;
		calculation_task& task;
		logging::logger& log;
		bool ready_check_on_exit = false;

		std::uint64_t operator()(std::uint64_t seed, std::uint64_t shots_per_thread);
	};
	struct init_data
	{
		logging::logger log{std::clog};
		std::mt19937_64 engine{0};
		std::size_t start_nthreads{0};
		bool slave, always_online;

		void init(int argc, const char* const* argv);
	};
	void read(std::istream& in, std::size_t nshapes, geometry::composition_t& composed);
	calculation_task& read_task(std::istream& in, std::map< std::size_t, calculation_task >& task_map);
	void send_task(logging::logger& log, init_data& data, calculation_task& task, auto& pool);
	void forget_finished(std::map< std::size_t, calculation_task >& task_map);
	bool register_death_callback(logging::logger* log);
}

template< class Typemap >
int mains::singleprocess::heavy(int argc, const char*const* argv)
{
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
	if (!data.slave)
	{
		return mains::singleprocess::light< Typemap >(argc, argv);
	}
	logging::logger& log = data.log;

	register_death_callback(&log);
	concurrent::multithread::pool< Typemap > pool;
	pool.resize(data.start_nthreads);
	pool.unlock();
	log.debug("initialized a pool with {} threads", data.start_nthreads);
	std::map< std::size_t, calculation_task > task_map;
	std::string open_key;
	log.info("slave init finished");
	while (std::cin >> open_key)
	{
		if (open_key != "<--open-task-->")
		{
			continue;
		}
		try
		{
			log.debug("clear finished tasks");
			forget_finished(task_map);
			log.debug("task reading started");
			calculation_task& tsk = read_task(std::cin, task_map);
			log.debug("task reading ended");
			send_task(log, data, tsk, pool);
		}
		catch (const std::exception& e)
		{
			log.fatal("failed to create task: {}", e.what());
			std::cin.clear(std::cin.rdstate() & ~std::ios::failbit);
		}
	}
	log.debug("slave started waiting for tasks completion");
	while (!task_map.empty())
	{
		forget_finished(task_map);
		std::this_thread::sleep_for(std::chrono::milliseconds{50});
	}
	log.info("slave finished");
	return 0;
}
template< class Typemap >
std::uint64_t mains::singleprocess::autosubmit_monothread< Typemap >::operator()(std::uint64_t seed,
			std::uint64_t shots_per_thread)
{
	std::uint64_t hits = monothread(seed, shots_per_thread, task.frame, task.composed);
	if (!ready_check_on_exit)
	{
		return hits;
	}
	std::unique_lock lock(pool);
	bool single_running = false;
	for (auto& i: task.results)
	{
		if (pool.task_state(i.first) != concurrent::multithread::task::state::closed)
		{
			if (single_running)
			{
				return hits;
			}
			single_running = true;
		}
	}
	std::uint64_t sum = hits;
	for (auto& i: task.results)
	{
		if (pool.task_state(i.first) == concurrent::multithread::task::state::closed)
		{
			sum += i.second.get();
		}
	}
	log.info("finished task #{} processing", task.id);
	task.success(sum);
	return hits;
}
void mains::singleprocess::send_task(logging::logger& log, init_data& data, calculation_task& tsk, auto& pool)
{
	autosubmit_monothread< decltype(pool) > executor{pool, tsk, log, data.always_online};
	std::uint64_t shots_per_thread = tsk.shots / tsk.nthreads;
	std::unique_lock lock(pool);
	if (pool.running() < tsk.nthreads)
	{
		pool.resize(tsk.nthreads);
		log.debug("thread pool extended to {} threads", tsk.nthreads);
	}
	for (std::size_t i = 1; i < tsk.nthreads; i++)
	{
		tsk.results.push_back(pool.create_task(executor, data.engine(), shots_per_thread));
	}
	shots_per_thread += tsk.shots % tsk.nthreads;
	if (data.always_online)
	{
		tsk.results.push_back(pool.create_task(executor, data.engine(), shots_per_thread));
		lock.unlock();
		log.info("task #{} started", tsk.id);
		return;
	}
	lock.unlock();
	log.debug("sended thread pool tasks");
	log.info("task #{} started", tsk.id);
	log.debug("started calculation on main thread");
	std::uint64_t hits = monothread(data.engine(), shots_per_thread, tsk.frame, tsk.composed);
	log.debug("finished calculation on main thread");
	for (auto& i: tsk.results)
	{
		hits += i.second.get();
	}
	log.info("finished task #{} processing", tsk.id);
	tsk.success(hits);
}
