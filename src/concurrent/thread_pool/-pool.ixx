export module concurrent.thread_pool:pool;

import <cstddef>;
import <map>;
import <list>;
import <atomic>;
import <exception>;
import <future>;
import <functional>;

namespace concurrent::multithread
{
	struct task
	{
		enum class state: char
		{
			created = 'W',
			planned = 'P',
			running = 'R',
			closed = 'F',
			unknown = 'U'
		};
		std::packaged_task< void() > executor;
		state status = state::created;
	};

	template< class F, class... Args >
	using task_bind_t = decltype(std::bind(std::declval< F >(), std::declval< Args >()...));
	template< class F, class... Args >
	using task_result_t = std::invoke_result_t< task_bind_t< F, Args... > >;

	/// Thread pool (necessary to use mutex for all interface calls except stop())
	export template< class Typemap, bool LightVersion = false >
	class pool: public Typemap::mutex
	{
	public:
		using idx_t = std::size_t;

		pool();
		~pool();
		pool(const pool&) = delete;
		pool& operator=(const pool&) = delete;

		template< class F, class... Args >
		std::pair< idx_t, std::future< task_result_t< F, Args... > > > create_task(F&& f, Args&&... args);
		bool cancel_task(idx_t id); ///< Cancel task if it hasn't started yet
		task::state task_state(idx_t id);

		void resize(std::size_t nthreads);
		idx_t queued() const; ///< not excluding cancelled
		std::size_t running() const;
		bool paused() const;
		void pause();
		void resume();
		bool stop();

	private:
		template< class Typemap, bool LightVersion > friend class worker;

		idx_t nregistered_tasks_ = 0;
		idx_t queue_head_idx_ = 0;
		std::map< idx_t, task > task_map_;
		Typemap::condition_variable new_tasks_cond_;

		std::size_t nregistered_threads_ = 0;
		std::size_t wished_nthreads_ = 0;
		std::map< std::size_t, std::thread > threads_;
		std::atomic_flag stop_flag_;
		std::atomic_flag pause_flag_;

		void create_thread();
		bool has_free_tasks() const noexcept;
	};
}

template< class Typemap, bool LightVersion >
concurrent::multithread::pool< Typemap, LightVersion >::pool()
{
	stop_flag_.clear(std::memory_order::release);
	pause_flag_.clear(std::memory_order::release);
	this->lock();
}
template< class Typemap, bool LightVersion >
concurrent::multithread::pool< Typemap, LightVersion >::~pool()
{
	if (!stop()) return;

	std::unique_lock lock(*this);
	for (auto it = threads_.begin(); it != threads_.end(); it++)
	{
		lock.unlock();
		it->second.join();
		lock.lock();
	}
	threads_.clear();
}
template< class Typemap, bool LightVersion >
bool concurrent::multithread::pool< Typemap, LightVersion >::cancel_task(idx_t id)
{
	if (task_state(id) == task::state::created)
	{
		task_map_.erase(task_map_.find(id));
		return true;
	}
	return false;
}
template< class Typemap, bool LightVersion >
concurrent::multithread::task::state concurrent::multithread::pool< Typemap, LightVersion >::task_state(idx_t id)
{
	if (id >= nregistered_tasks_) return task::state::unknown;
	auto it = task_map_.find(id);
	if (it == task_map_.end()) return task::state::closed;
	return it->second.status;
}
template< class Typemap, bool LightVersion >
concurrent::multithread::pool< Typemap, LightVersion >::idx_t concurrent::multithread::pool< Typemap, LightVersion >::queued() const
{
	return nregistered_tasks_ - queue_head_idx_;
}
template< class Typemap, bool LightVersion >
std::size_t concurrent::multithread::pool< Typemap, LightVersion >::running() const
{
	return threads_.size();
}
template< class Typemap, bool LightVersion >
bool concurrent::multithread::pool< Typemap, LightVersion >::paused() const
{
	return pause_flag_.test(std::memory_order::acquire);
}
template< class Typemap, bool LightVersion >
void concurrent::multithread::pool< Typemap, LightVersion >::pause()
{
	pause_flag_.test_and_set(std::memory_order::release);
}
template< class Typemap, bool LightVersion >
void concurrent::multithread::pool< Typemap, LightVersion >::resume()
{
	pause_flag_.clear(std::memory_order::release);
	new_tasks_cond_.notify_all();
}
template< class Typemap, bool LightVersion >
bool concurrent::multithread::pool< Typemap, LightVersion >::stop()
{
	if (stop_flag_.test_and_set(std::memory_order::acquire))
	{
		return false;
	}
	new_tasks_cond_.notify_all();
	pause_flag_.clear(std::memory_order::release);
	return true;
}
template< class Typemap, bool LightVersion >
template< class F, class... Args >
std::pair< typename concurrent::multithread::pool< Typemap, LightVersion >::idx_t,
			std::future< concurrent::multithread::task_result_t< F, Args... > > >
		concurrent::multithread::pool< Typemap, LightVersion >::create_task(F&& f, Args&&... args)
{
	using result_t = task_result_t< F, Args... >;
	struct executor
	{
		std::packaged_task< result_t() > ptask;
		void operator()()
		{ ptask(); }
	};
	std::packaged_task< result_t() > pack(std::bind(std::forward< F >(f), std::forward< Args >(args)...));
	std::pair< idx_t, std::future< result_t > > result = std::make_pair(nregistered_tasks_, pack.get_future());
	task_map_.try_emplace(nregistered_tasks_++, task{std::packaged_task< void() >(executor{std::move(pack)})});
	new_tasks_cond_.notify_all();
	return result;
}
template< class Typemap, bool LightVersion >
void concurrent::multithread::pool< Typemap, LightVersion >::create_thread()
{
	auto it = threads_.try_emplace(nregistered_threads_++);
	it.first->second = std::thread{worker{*this, it.first}};
}
template< class Typemap, bool LightVersion >
void concurrent::multithread::pool< Typemap, LightVersion >::resize(std::size_t nthreads)
{
	if (nthreads == running()) return;
	if (nthreads > running())
	{
		for (std::size_t i = nthreads - running(); i > 0; i--)
		{
			create_thread();
		}
	}
	wished_nthreads_ = nthreads;
}
template< class Typemap, bool LightVersion >
bool concurrent::multithread::pool< Typemap, LightVersion >::has_free_tasks() const noexcept
{
	return queue_head_idx_ < nregistered_tasks_;
}
