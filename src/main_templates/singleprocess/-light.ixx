export module main_templates.singleprocess:light;

import <iostream>;
import <print>;
import <random>;
import <future>;
import <vector>;
import <string>;

import geometry;
import concurrent.thread_pool;

import :common;

namespace mains::singleprocess
{
	export template< class Typemap >
	int light(int argc, const char* const* argv)
	{
		if (argc < 2)
		{
			std::println(std::cerr, "not enough plain arguments");
			return 1;
		}
		std::uint64_t shots = 1;
		std::uint64_t seed = 0;
		try
		{
			shots = std::stoull(argv[1]);
			if (argc == 3)
			{
				seed = std::stoull(argv[2]);
			}
		}
		catch (const std::exception& e)
		{
			std::println(std::cerr, "failed to parse arguments: {}", e.what());
			return 1;
		}
		if (shots == 0)
		{
			std::println(std::cerr, "zero shots");
			return 1;
		}
		if (argc > 3)
		{
			std::println(std::cerr, "too many plain arguments");
			return 1;
		}

		std::mt19937_64 engine(seed);
		geometry::circle_t circle{0, {0, 0}};
		ptrdiff_t nthreads = 1;
		if (!(std::cin >> circle.radius >> nthreads))
		{
			std::println(std::cerr, "failed to read radius & threads count");
			return 2;
		}
		if (circle.radius <= 0.0f)
		{
			std::println(std::cerr, "circle radius cannot be nonpositive");
			return 2;
		}
		if (nthreads < 1)
		{
			std::println(std::cerr, "threads count cannot be less then 1");
			return 2;
		}
		geometry::rect_t frame = geometry::frame_of(circle);
		geometry::composition_t composed;
		composed.shapes.emplace_back(geometry::shape_t{circle});

		std::vector< std::future< std::uint64_t > > results;
		results.reserve(nthreads);
		std::uint64_t hits = 0;

		concurrent::multithread::pool< Typemap > pool;
		pool.resize(nthreads - 1);
		std::uint64_t shots_per_thread = shots / nthreads;
		for (; nthreads > 1; nthreads--)
		{
			results.push_back(pool.create_task(monothread,
							engine(), shots_per_thread, frame, std::ref(composed)).second);
		}
		pool.unlock();

		shots_per_thread += shots % (results.size() + 1);
		hits = monothread(engine(), shots_per_thread, frame, composed);
		for (std::future< std::uint64_t >& future: results)
		{
			hits += future.get();
		}
		std::println("{}", shots_to_square(frame, shots, hits));
		return 0;
	}
}
