export module geometry:basic_operations;

import <cstdint>;
import <algorithm>;
import <numbers>;
import <cmath>;

import :shapes;

namespace geometry
{
	export constexpr float distance_sqr(point_t p1, point_t p2);
	export constexpr rect_t merge_frames(rect_t lhs, rect_t rhs);
	export constexpr inline point_t rotate(point_t center, point_t p, float angle);
	export constexpr point_t rotate(point_t center, point_t p, float sin, float cos);

	export constexpr float square_of(rect_t r);
	export constexpr float square_of(circle_t c);
	export constexpr float square_of(ellipse_t e);
	export template< class T >
	constexpr float square_of(rotated_t< T > shape);

	export constexpr point_t center_of(rect_t r);
	export constexpr point_t center_of(circle_t c);
	export constexpr point_t center_of(ellipse_t e);
	export template< class T >
	constexpr point_t center_of(rotated_t< T > shape);

	export constexpr rect_t frame_of(rect_t r);
	export constexpr rect_t frame_of(circle_t c);
	export constexpr rect_t frame_of(ellipse_t e);
	export template< class T >
	constexpr rect_t frame_of(rotated_t< T > shape);

	export constexpr bool inside_of(point_t p, rect_t r);
	export constexpr bool inside_of(point_t p, circle_t c);
	export constexpr bool inside_of(point_t p, ellipse_t e);
	export template< class T >
	constexpr bool inside_of(point_t p, rotated_t< T > shape);
}

constexpr float geometry::distance_sqr(point_t p1, point_t p2)
{
	p1.x -= p2.x;
	p1.y -= p2.y;
	return p1.x * p1.x + p1.y * p1.y;
}
constexpr geometry::rect_t geometry::merge_frames(rect_t lhs, rect_t rhs)
{
	return {{std::min(lhs.p1.x, rhs.p1.x), std::min(lhs.p1.y, rhs.p1.y)},
				{std::max(lhs.p2.x, rhs.p2.x), std::max(lhs.p2.y, rhs.p2.y)}};
}
constexpr inline geometry::point_t geometry::rotate(point_t center, point_t p, float angle)
{
	return rotate(center, p, std::sin(angle), std::cos(angle));
}
constexpr geometry::point_t geometry::rotate(point_t center, point_t p, float sin, float cos)
{
	p.x -= center.x;
	p.y -= center.y;
	return {center.x + (p.x * cos - p.y * sin), center.x + (p.x * sin + p.y * cos)};
}

constexpr float geometry::square_of(rect_t rect)
{
	rect.p1.x -= rect.p2.x;
	rect.p1.y -= rect.p2.y;
	return rect.p1.x * rect.p1.y;
}
constexpr float geometry::square_of(circle_t c)
{
	return std::numbers::pi_v< float > * c.radius * c.radius;
}
constexpr float geometry::square_of(ellipse_t e)
{
	return std::numbers::pi_v< float > * e.v_semiaxis * e.h_semiaxis;
}
template< class T >
constexpr float geometry::square_of(rotated_t< T > shape)
{
	return square_of(shape.shape);
}

constexpr geometry::point_t geometry::center_of(rect_t r)
{
	return {(r.p1.x + r.p2.x) / 2, (r.p1.y + r.p2.y) / 2};
}
constexpr geometry::point_t geometry::center_of(circle_t c)
{
	return c.center;
}
constexpr geometry::point_t geometry::center_of(ellipse_t e)
{
	return e.center;
}
template< class T >
constexpr geometry::point_t geometry::center_of(rotated_t< T > shape)
{
	return center_of(shape.shape);
}

constexpr geometry::rect_t geometry::frame_of(rect_t r)
{
	return r;
}
constexpr geometry::rect_t geometry::frame_of(circle_t c)
{
	return {{c.center.x - c.radius, c.center.y - c.radius}, {c.center.x + c.radius, c.center.y + c.radius}};
}
constexpr geometry::rect_t geometry::frame_of(ellipse_t e)
{
	return {{e.center.x - e.h_semiaxis, e.center.y - e.v_semiaxis},
				{e.center.x + e.h_semiaxis, e.center.y + e.v_semiaxis}};
}
template< class T >
constexpr geometry::rect_t geometry::frame_of(rotated_t< T > shape)
{
	rect_t frame = frame_of(shape.shape);
	point_t center = center_of(frame);
	float cos = std::cosf(shape.angle);
	float sin = std::sinf(shape.angle);

	point_t p1 = rotate(center, frame.p1, sin, cos);
	point_t p2 = rotate(center, frame.p2, sin, cos);
	point_t p3 = rotate(center, {frame.p1.x, frame.p2.y}, sin, cos);
	point_t p4 = rotate(center, {frame.p2.x, frame.p1.y}, sin, cos);
	return {{std::min({p1.x, p2.x, p3.x, p4.x}), std::min({p1.y, p2.y, p3.y, p4.y})},
				{std::max({p1.x, p2.x, p3.x, p4.x}), std::max({p1.y, p2.y, p3.y, p4.y})}};
}

constexpr bool geometry::inside_of(point_t p, rect_t r)
{
	return (p.x >= r.p1.x) && (p.x <= r.p2.x) && (p.y >= r.p1.y) && (p.y <= r.p2.y);
}
constexpr bool geometry::inside_of(point_t p, circle_t c)
{
	return distance_sqr(p, c.center) <= c.radius * c.radius;
}
constexpr bool geometry::inside_of(point_t p, ellipse_t e)
{
	p.x -= e.center.x;
	p.y -= e.center.y;
	return (p.x * p.x) / (e.h_semiaxis * e.h_semiaxis) + (p.y * p.y) / (e.v_semiaxis * e.v_semiaxis) <= 1;
}
template< class T >
constexpr bool geometry::inside_of(point_t p, rotated_t< T > shape)
{
	return inside_of(rotate(center_of(shape), p, shape.angle), shape.shape);
}
