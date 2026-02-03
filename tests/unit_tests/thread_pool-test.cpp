#include <boost/test/unit_test.hpp>

import <thread>;
import <exception>;

import concurrent.thread_pool;
import concurrent.type_suppliers;

BOOST_AUTO_TEST_SUITE(PSC_thread_pool)

BOOST_AUTO_TEST_CASE(tasks_queue_test)
{
	struct test_task
	{
		bool generate_exception = false;
		std::size_t operator()(std::size_t i, std::chrono::milliseconds delay)
		{
			std::this_thread::sleep_for(delay);
			if (generate_exception)
			{
				throw std::runtime_error("spec_msg");
			}
			return i;
		}
	};
	concurrent::multithread::pool< concurrent::std_types > pool;
	pool.resize(8);
	pool.unlock();

	std::this_thread::sleep_for(std::chrono::milliseconds{100});
	BOOST_TEST(pool.running() == 8);
	BOOST_TEST(pool.queued() == 0);
	pool.lock();
	constexpr std::size_t n_nothrow_tasks = 500;
	std::pair< std::size_t, std::future< std::size_t > > tasks_results[n_nothrow_tasks + 1];
	for (std::size_t i = n_nothrow_tasks; i > 0; i--)
	{
		tasks_results[i] = pool.create_task(test_task{false}, i, std::chrono::milliseconds{10});
	}
	tasks_results[0] = pool.create_task(test_task{true}, 1000, std::chrono::milliseconds{10});
	pool.unlock();

	for (std::size_t i = n_nothrow_tasks; i > 0; i--)
	{
		BOOST_TEST(tasks_results[i].second.get() == i);
	}
	BOOST_CHECK_THROW(tasks_results[0].second.get(), std::runtime_error);
	BOOST_TEST(pool.running() == 8);
	BOOST_TEST(pool.queued() == 0);

	pool.lock();
	pool.resize(4);
	for (std::size_t i = 8; i > 0; i--)
	{
		tasks_results[i] = pool.create_task(test_task{false}, i, std::chrono::milliseconds{200});
	}
	pool.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds{100});
	BOOST_TEST(pool.running() == 4);
	BOOST_TEST(pool.queued() == 4);
	pool.lock();
	pool.pause();
	pool.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});
	BOOST_TEST(pool.running() == 4);
	BOOST_TEST(pool.queued() == 4);
	pool.lock();
	pool.resume();
	pool.unlock();
}
BOOST_AUTO_TEST_CASE(stop_paused_test)
{
	concurrent::multithread::pool< concurrent::std_types > pool;
	pool.resize(8);
	pool.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds{100});
	pool.lock();
	pool.pause();
	pool.unlock();
}

BOOST_AUTO_TEST_SUITE_END()
