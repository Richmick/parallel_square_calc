export module concurrent.process:process;

import <string>;
import <chrono>;
import <span>;

import os;

namespace concurrent::multiprocess
{
	/// Process wrapper
	/**
	  * Requires to setup interprocess communication before calling start()
	  * By default destructor calls close()
	  */
	export class process
	{
	public:
		enum class IO
		{
			in,
			out,
			err
		};

		process() = default;
		~process();

		void start(std::string path, std::span< const std::string > arguments);
		void set_io(IO stream, os::pipe pipe);
		os::pipe& get_io(IO stream);

		void close();
		void kill();

		void wait();
		bool wait_for(std::chrono::milliseconds limit);
		void detach();
		bool alive() const;

	private:
		os::unique_handle handle_;
		os::pipe stdin_, stdout_, stderr_;
	};
}
