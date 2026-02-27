module main_templates.multiprocess;

import <print>;

void mains::multiprocess::data_visitor::operator()(geometry::circle_t c)
{
	std::print(out, "{} {} {}", c.radius, c.center.x, c.center.y);
}
void mains::multiprocess::data_visitor::operator()(geometry::rect_t r)
{
	std::print(out, "{} {} {} {}", r.p1.x, r.p1.y, r.p2.x, r.p2.y);
}
void mains::multiprocess::data_visitor::operator()(geometry::ellipse_t e)
{
	std::print(out, "{} {} {} {}", e.h_semiaxis, e.v_semiaxis, e.center.x, e.center.y);
}
void mains::multiprocess::name_visitor::operator()(const geometry::circle_t&)
{
	std::print(out, "circle");
}
void mains::multiprocess::name_visitor::operator()(const geometry::rect_t&)
{
	std::print(out, "rect");
}
void mains::multiprocess::name_visitor::operator()(const geometry::ellipse_t&)
{
	std::print(out, "ellipse");
}

void mains::multiprocess::cmd_controller::add_shape()
{
	std::string name, type;
	std::cin >> name >> type;
	auto ins = shapes_.try_emplace(name);
	if (!ins.second)
	{
		throw std::invalid_argument("entered existing name");
	}
	if (type == "circle")
	{
		geometry::circle_t c;
		std::cin >> c.radius >> c.center.x >> c.center.y;
		ins.first->second = c;
	}
	else if (type == "rect")
	{
		geometry::rect_t r;
		std::cin >> r.p1.x >> r.p1.y >> r.p2.x >> r.p2.y;
		ins.first->second = r;
	}
	else if (type == "ellipse")
	{
		geometry::ellipse_t e;
		std::cin >> e.h_semiaxis >> e.v_semiaxis >> e.center.x >> e.center.y;
		ins.first->second = e;
	}
	else
	{
		throw std::invalid_argument("entered unknown shape type");
	}
}
void mains::multiprocess::cmd_controller::print_shape()
{
	std::string name;
	std::cin >> name;
	geometry::shape_t shape = shapes_.at(name);
	std::visit(name_visitor{std::cout}, shape);
	std::print(" ");
	std::visit(data_visitor{std::cout}, shape);
	std::println();
}
void mains::multiprocess::cmd_controller::print_all_shapes()
{
	if (shapes_.empty())
	{
		std::println("<no shapes created>");
		return;
	}
	for (const auto& [name, shape] : shapes_)
	{
		std::print("* {} = ", name);
		std::visit(name_visitor{std::cout}, shape);
		std::print(" ");
		std::visit(data_visitor{std::cout}, shape);
		std::println();
	}
}
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
void mains::multiprocess::cmd_controller::add_composition()
{
	std::string name;
	std::cin >> name;
	auto ins = compositions_.try_emplace(name);
	if (!ins.second)
	{
		throw std::invalid_argument("entered existing name");
	}
	geometry::composition_t& composed = ins.first->second;
	float angle;
	while (std::cin >> name, name != ";")
	{
		std::cin >> angle;
		composed.shapes.push_back({shapes_.at(name), angle});
	}
}
void mains::multiprocess::cmd_controller::print_composition()
{
	std::string name;
	std::cin >> name;
	geometry::composition_t& composed = compositions_.at(name);
	for (auto shape: composed.shapes)
	{
		std::print("* rotated by {} radians ", shape.angle);
		std::visit(name_visitor{std::cout}, shape.shape);
		std::print(" ");
		std::visit(data_visitor{std::cout}, shape.shape);
		std::println();
	}
}
void mains::multiprocess::cmd_controller::print_compositions_names()
{
	if (compositions_.empty())
	{
		std::println("<no compositions created>");
		return;
	}
	for (const auto& [name, composition]: compositions_)
	{
		std::println("* {}", name);
	}
}
