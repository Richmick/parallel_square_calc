export module geometry:composition;

import <variant>;
import <vector>;

import :shapes;
import :basic_operations;

namespace geometry
{
	export using shape_t = std::variant< rect_t, circle_t, ellipse_t >;
	export struct composition_t
	{
		std::vector< rotated_t< shape_t > > shapes; ///< Must be not empty
	};

	export constexpr float square_of(shape_t shape);
	export constexpr point_t center_of(shape_t shape);
	export constexpr rect_t frame_of(shape_t shape);
	export constexpr rect_t frame_of(const composition_t& c);
	export constexpr bool inside_of(point_t p, shape_t shape);
	export constexpr bool inside_of(point_t p, const composition_t& c);
}

constexpr float geometry::square_of(shape_t shape)
{
	return std::visit([](auto s){ return square_of(s); }, shape);
}
constexpr geometry::point_t geometry::center_of(shape_t shape)
{
	return std::visit([](auto s){ return center_of(s); }, shape);
}
constexpr geometry::rect_t geometry::frame_of(shape_t shape)
{
	return std::visit([](auto s){ return frame_of(s); }, shape);
}
constexpr geometry::rect_t geometry::frame_of(const composition_t& c)
{
	rect_t result = frame_of(c.shapes.front());
	for (const rotated_t< shape_t >& i: c.shapes)
	{
		result = merge_frames(result, frame_of(i));
	}
	return result;
}
constexpr bool geometry::inside_of(point_t p, shape_t shape)
{
	return std::visit([p](auto s){ return inside_of(p, s); }, shape);
}
constexpr bool geometry::inside_of(point_t p, const composition_t& c)
{
	for (const rotated_t< shape_t >& i: c.shapes)
	{
		if (inside_of(p, i))
		{
			return true;
		}
	}
	return false;
}
