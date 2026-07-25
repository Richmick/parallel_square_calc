module;
#ifdef __linux__
	#include <unistd.h>
#endif
export module os.linux:common;

#ifdef __linux__

namespace linux
{
	using handle_t = int;
	using ::pid_t;
}

#endif
