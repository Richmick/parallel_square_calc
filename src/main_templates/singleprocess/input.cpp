module main_templates.singleprocess;

import flags;

void mains::singleprocess::init_data::init(int argc, const char* const* argv)
{
	dispatch::expectation expect;
	expect.add_long_flag("slave", 'S'); // not use light version
	expect.add_long_flag("always-online", 'a');
	expect.add_long_flag("debug");
	expect.add_long_flag("info");
	expect.add_long_flag("fatal");
	expect.add_long_flag("silent");
	expect.add_key_value("seed");
	expect.add_key_value("id");
	expect.add_key_value("start-threads");
	dispatch::flags flags({argv, static_cast< std::size_t >(argc)}, expect);
	slave = flags.test('S');
	if (!slave)
	{
		return;
	}
	if (flags.plain().size() > 1)
	{
		throw std::invalid_argument("no plain arguments expected");
	}
	if (flags.has_key("id"))
	{
		log.id(std::stoul(flags["id"].data()));
	}
	if (flags.has_key("seed"))
	{
		engine.seed(std::stoull(flags["seed"].data()));
	}
	if (flags.has_key("start-threads"))
	{
		start_nthreads = std::stoull(flags["start-threads"].data());
	}

	bool silent = flags.test("silent");
	bool fatal = flags.test("fatal");
	bool info = flags.test("info");
	bool debug = flags.test("debug");
	if (silent + fatal + info + debug > 1)
	{
		throw std::invalid_argument("many log levels were chosen");
	}
	if (fatal) log.level(logging::level::fatal);
	if (info) log.level(logging::level::info);
	if (debug) log.level(logging::level::debug);
	log.debug("logger init finished");

	always_online = flags.test('a');
	log.debug("startup argumments parsing finished");
}
void mains::singleprocess::read(std::istream& in, std::size_t nshapes, geometry::composition_t& composed)
{
	for (std::size_t i = 0; i < nshapes; i++)
	{
		std::string shape_name;
		geometry::rotated_t< geometry::shape_t > shape;
		if (!(in >> shape_name >> shape.angle))
		{
			throw std::ios_base::failure(std::format("failed to read shape name & rotation (shape #{})", i + 1));
		}
		if (shape_name == "circle")
		{
			geometry::circle_t c;
			if (!(in >> c.radius >> c.center.x >> c.center.y))
			{
				throw std::ios_base::failure(std::format("failed to read circle data (shape #{})", i + 1));
			}
			if (c.radius <= 0)
			{
				throw std::invalid_argument(std::format("nonpositive circle radius (shape #{})", i + 1));
			}
			shape.shape = c;
		}
		else if (shape_name == "rect")
		{
			geometry::rect_t r;
			if (!(in >> r.p1.x >> r.p1.y >> r.p2.x >> r.p2.y))
			{
				throw std::ios_base::failure(std::format("failed to read rectangle data (shape #{})", i + 1));
			}
			if ((r.p1.x >= r.p2.x) || (r.p1.y >= r.p2.y))
			{
				throw std::invalid_argument(std::format("invalid rectangle (shape #{})", i + 1));
			}
			shape.shape = r;
		}
		else if (shape_name == "ellipse")
		{
			geometry::ellipse_t e;
			if (!(in >> e.h_semiaxis >> e.v_semiaxis >> e.center.x >> e.center.y))
			{
				throw std::ios_base::failure(std::format("failed to read ellipse data (shape #{})", i + 1));
			}
			if ((e.h_semiaxis <= 0) || (e.v_semiaxis <= 0))
			{
				throw std::invalid_argument(std::format("nonpositive ellipse semiaxis (shape #{})", i + 1));
			}
			shape.shape = e;
		}
		else
		{
			throw std::invalid_argument(std::format("unknown shape name (shape #{})", i + 1));
		}
		composed.shapes.push_back(shape);
	}
}
mains::singleprocess::calculation_task& mains::singleprocess::read_task(std::istream& in,
			std::map< std::size_t, calculation_task >& task_map)
{
	std::size_t id = 0;
	if (!(std::cin >> id))
	{
		throw std::ios_base::failure("failed to read task id");
	}
	auto insert_response = task_map.try_emplace(id);
	if (!insert_response.second)
	{
		throw std::invalid_argument(std::format("task #{} already exists", id));
	}
	calculation_task& tsk = insert_response.first->second;
	tsk.id = id;

	std::size_t nshapes = 0;
	if (!(std::cin >> tsk.nthreads >> tsk.shots >> nshapes))
	{
		tsk.fatal();
		throw std::invalid_argument("failed to read task description");
	}
	if (tsk.nthreads < 1)
	{
		tsk.fatal();
		throw std::invalid_argument("invalid thread count");
	}
	if (tsk.shots < 1)
	{
		tsk.fatal();
		throw std::invalid_argument("invalid shots count");
	}
	if (nshapes < 1)
	{
		tsk.fatal();
		throw std::invalid_argument("invalid shapes count");
	}
	tsk.results.reserve(tsk.nthreads);
	tsk.composed.shapes.reserve(nshapes);
	try
	{
		read(std::cin, nshapes, tsk.composed);
	}
	catch (...)
	{
		tsk.fatal();
		throw;
	}
	tsk.frame = geometry::frame_of(tsk.composed);
	return tsk;
}
void mains::singleprocess::forget_finished(std::map< std::size_t, calculation_task >& task_map)
{
	auto i = task_map.begin();
	while ((i != task_map.end()) && i->second.finished)
	{
		task_map.erase(i++);
	}
}
