export module concurrent.thread_pool:worker;

import :pool;

import <map>;
import <mutex>;
import <atomic>;

namespace concurrent::multithread
{
	export template< class Typemap >
	class worker
	{
	public:
		using pool_type = pool< Typemap >;
		using self_iterator = typename std::map< std::size_t, typename Typemap::thread >::iterator;
		using task_iterator = typename std::map< typename pool_type::idx_t, task >::iterator;

		worker(pool_type& p, self_iterator self):
			pool_(p),
			self_it_(self),
			queued_(p.task_map_.end())
		{}
		void operator()()
		{
			while (true)
			{
				std::unique_lock lock(static_cast< Typemap::mutex& >(pool_));
				if (queued_ != pool_.task_map_.end())
				{
					pool_.task_map_.erase(queued_);
					queued_ = pool_.task_map_.end();
				}
				if (!need_stop() && pause())
				{
					pool_.new_tasks_cond_.wait(lock, [this]{ return need_stop() || !pause(); });
				}
				if (!stop_flag_ && !try_update_tasks())
				{
					pool_.new_tasks_cond_.wait(lock,
								[this]{ return need_stop() || (!pause() && try_update_tasks()); });
				}
				if (stop_flag_)
				{
					if (!not_erase_self_)
					{
						self_it_->second.detach();
						pool_.threads_.erase(self_it_);
					}
					return;
				}
				lock.unlock();
				task& tsk = queued_->second;
				tsk.status = task::state::running;
				tsk.executor();
				tsk.status = task::state::closed;
			}
		}
	private:
		pool_type& pool_;
		self_iterator self_it_;
		task_iterator queued_;
		bool stop_flag_ = false, not_erase_self_ = false;

		bool need_stop()
		{
			not_erase_self_ = pool_.stop_flag_.test(std::memory_order::acquire);
			stop_flag_ = (pool_.wished_nthreads_ < pool_.running()) || not_erase_self_;
			return stop_flag_;
		}
		bool pause()
		{
			return pool_.pause_flag_.test(std::memory_order::acquire);
		}
		bool try_update_tasks()
		{
			if (!pool_.has_free_tasks())
			{
				return false;
			}
			queued_ = pool_.task_map_.find(pool_.queue_head_idx_);
			queued_->second.status = task::state::planned;
			pool_.inc_head();
			return true;
		}
	};
}
