module main_templates.multiprocess;

import <print>;

void mains::multiprocess::cmd_controller::calculate_shape_frame()
{
	std::string name;
	std::cin >> name;
	const geometry::shape_t& shape = shapes_.at(name);
	geometry::rect_t frame = geometry::frame_of(shape);
	std::println("{{{}x {}y}}, {{{}x {}y}}", frame.p1.x, frame.p1.y, frame.p2.x, frame.p2.y);
}
void mains::multiprocess::cmd_controller::calculate_composition_frame()
{
	std::string name;
	std::cin >> name;
	const geometry::composition_t& composed = compositions_.at(name);
	geometry::rect_t frame = geometry::frame_of(composed);
	std::println("{{{}x {}y}}, {{{}x {}y}}", frame.p1.x, frame.p1.y, frame.p2.x, frame.p2.y);
}
void mains::multiprocess::cmd_controller::print_compositions_names()
{
	bool any = false;
	for (const auto& [name, composition]: compositions_)
	{
		std::println("* {}", name);
		any = true;
	}
	if (!any)
	{
		std::println("<no shapes created>");
	}
}
