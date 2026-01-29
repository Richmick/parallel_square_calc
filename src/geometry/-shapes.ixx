export module geometry:shapes;

namespace geometry
{
	export struct point_t
	{
		float x, y;
	};
	export struct rect_t
	{
		point_t p1, p2;
	};
	export struct circle_t
	{
		float radius;
		point_t center;
	};
	export struct ellipse_t
	{
		float v_semiaxis;
		float h_semiaxis;
		point_t center;
	};
	export template< class T >
	struct rotated_t
	{
		T shape;
		float angle;
	};
}
