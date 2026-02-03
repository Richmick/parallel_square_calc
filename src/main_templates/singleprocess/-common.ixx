export module main_templates.singleprocess:common;

import <random>;
import geometry;

namespace mains
{
	std::uint64_t monothread(std::uint64_t seed, std::uint64_t shots,
				geometry::rect_t frame, geometry::composition_t& composed)
	{
		std::mt19937_64 engine(seed);
		std::uniform_real_distribution< float > x_distr(frame.p1.x, frame.p2.x);
		std::uniform_real_distribution< float > y_distr(frame.p1.y, frame.p2.y);
		std::uint64_t result = 0;
		for (; shots > 0; shots--)
		{
			result += geometry::inside_of({x_distr(engine), y_distr(engine)}, composed);
		}
		return result;
	}
	float shots_to_square(geometry::rect_t frame, std::uint64_t shots, std::uint64_t hits)
	{
		return geometry::square_of(frame) * hits / shots;
	}
}
