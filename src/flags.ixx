export module flags;

import <span>;
import <map>;
import <set>;
import <vector>;
import <string_view>;
import <string>;

namespace dispatch
{
	/// Restriction ruleset for flags & key-values
	export class expectation
	{
	public:
		enum class match_t : char
		{
			empty = '\0',
			key_value = ' ',
			value_after_key = '\t',
		};

		/// Expect arguments like "--key=value"
		void add_key_value(std::string key) &;
		/// Expect flags like "--name"
		void add_long_flag(std::string name, char synonym = '\0') &;
		/// Expect "value following key" arguments like "key value" ("--key" doesn't required)
		void add_value_after_key(std::string key) &;
		/// Expect monosymbolic flags like -c (connects like "-abcd")
		void add_monoflag(char c) &;

		bool test(char c) const;
		std::pair< match_t, bool > test(std::string_view key) const;
	private:
		std::string monoflags_;
		std::set< std::pair< std::string, match_t >, std::less<> > expected_;
	};

	/// Non-owning key-value & flags parser
	/**
	 * Scans for --key=value, key value, --flag, -abcd (-a, -b, -c, -d)
	 * Saves nonkey arguments to std::vector
	 * Stops searching flags/keys on "--" (flushes remaining arguments as nonkey)
	 * \throw std::out_of_range When value don't follow plain key (not "--key=")
	 * \throw std::invalid_argument When expectations were broken or format violated
	 */
	export class flags
	{
	public:
		flags() = default;
		flags(std::span< const char*const > args);
		flags(std::span< const char*const > args, const expectation& expected);

		void parse(std::span< const char*const > args) &;
		void parse(std::span< const char*const > args, const expectation& expected) &;

		std::span< const std::string_view > plain() const&;
		std::string_view operator[](std::string_view key) const&;
		bool has_key(std::string_view key) const;
		bool test(std::string_view flag) const;
		bool test(char c) const;

		void clear() &;
	private:
		std::set< std::string_view > flags_;
		std::map< std::string_view, std::string_view > map_;
		std::string monoflags_;
		std::vector< std::string_view > plain_args_;

		void add_monoflag(char c);
	};
}

module: private;

import <cctype>;
import <stdexcept>;
import <algorithm>;

void dispatch::expectation::add_key_value(std::string key) &
{
	expected_.emplace(std::move(key), match_t::key_value);
}
void dispatch::expectation::add_long_flag(std::string name, char synonym) &
{
	expected_.emplace(std::move(name), static_cast< match_t >(synonym));
	add_monoflag(synonym);
}
void dispatch::expectation::add_value_after_key(std::string key) &
{
	expected_.emplace(std::move(key), match_t::value_after_key);
}
void dispatch::expectation::add_monoflag(char c) &
{
	if ((c == '\0') || std::isspace(static_cast< unsigned char >(c)))
	{
		return;
	}
	auto ins_point = std::lower_bound(monoflags_.begin(), monoflags_.end(), c);
	if ((ins_point == monoflags_.end()) || (*ins_point != c))
	{
		monoflags_.insert(ins_point, c);
	}
}
bool dispatch::expectation::test(char c) const
{
	auto pos = std::lower_bound(monoflags_.begin(), monoflags_.end(), c);
	return (pos != monoflags_.end()) && (*pos == c);
}
std::pair< dispatch::expectation::match_t, bool > dispatch::expectation::test(std::string_view key) const
{
	auto pos = expected_.lower_bound(std::make_pair(key, match_t::empty));
	if ((pos != expected_.end()) && (pos->first == key))
	{
		return {pos->second, true};
	}
	return {match_t::empty, false};
}

dispatch::flags::flags(std::span< const char*const > args)
{
	parse(args);
}
dispatch::flags::flags(std::span< const char*const > args, const expectation& expected)
{
	parse(args, expected);
}

void dispatch::flags::parse(std::span< const char*const > args) &
{
	for (std::size_t i = 0; i < args.size(); i++)
	{
		std::string_view flag(args[i]);
		if (args[i][0] == '-')
		{
			if (args[i][1] == '-')
			{
				flag.remove_prefix(2);
				if (flag.size() == 0)
				{
					while (++i < args.size())
					{
						plain_args_.push_back(args[i]);
					}
					return;
				}
				std::string::size_type mid = flag.find('=');
				if (mid != std::string::npos)
				{
					if (mid == 0)
					{
						throw std::invalid_argument("empty key (--=value)");
					}
					if (!map_.try_emplace(flag.substr(0, mid), flag.substr(mid + 1)).second)
					{
						throw std::invalid_argument("found key-value duplicate");
					}
					continue;
				}
				flags_.insert(flag);
				continue;
			}
			flag.remove_prefix(1);
			for (char c: flag)
			{
				add_monoflag(c);
			}
			continue;
		}
		plain_args_.push_back(flag);
	}
}
void dispatch::flags::parse(std::span< const char*const > args, const expectation& expected) &
{
	for (std::size_t i = 0; i < args.size(); i++)
	{
		std::string_view flag(args[i]);
		if (expected.test(flag).first == expectation::match_t::value_after_key)
		{
			if (i + 1 == args.size())
			{
				throw std::out_of_range("found key preceding value at the end of the args sequence");
			}
			if (!map_.try_emplace(flag, args[++i]).second)
			{
				throw std::invalid_argument("found key-value duplicate");
			}
			continue;
		}
		if (args[i][0] == '-')
		{
			if (args[i][1] == '-')
			{
				flag.remove_prefix(2);
				if (flag.size() == 0)
				{
					while (++i < args.size())
					{
						plain_args_.push_back(args[i]);
					}
					return;
				}
				std::string::size_type mid = flag.find('=');
				if (mid != std::string::npos)
				{
					if (mid == 0)
					{
						throw std::invalid_argument("empty key (--=value)");
					}
					if (expected.test(flag.substr(0, mid)).first != expectation::match_t::key_value)
					{
						throw std::invalid_argument("unknown key");
					}
					if (!map_.try_emplace(flag.substr(0, mid), flag.substr(mid + 1)).second)
					{
						throw std::invalid_argument("found key-value duplicate");
					}
					continue;
				}
				auto flag_test = expected.test(flag);
				if (!flag_test.second)
				{
					throw std::invalid_argument("unknown flag");
				}
				if ((flag_test.first == expectation::match_t::key_value)
							|| (flag_test.first == expectation::match_t::value_after_key))
				{
					throw std::invalid_argument("invalid key-value pair");
				}
				flags_.insert(flag);
				add_monoflag(static_cast< char >(flag_test.first));
				continue;
			}
			flag.remove_prefix(1);
			for (char c: flag)
			{
				if (!expected.test(c))
				{
					throw std::invalid_argument("invalid monoflag");
				}
				add_monoflag(c);
			}
			continue;
		}
		plain_args_.push_back(flag);
	}
}

std::span< const std::string_view > dispatch::flags::plain() const&
{
	return {plain_args_.data(), plain_args_.size()};
}
std::string_view dispatch::flags::operator[](std::string_view key) const&
{
	return map_.at(key);
}
bool dispatch::flags::has_key(std::string_view key) const
{
	return map_.contains(key);
}
bool dispatch::flags::test(std::string_view flag) const
{
	return flags_.contains(flag);
}
bool dispatch::flags::test(char c) const
{
	auto pos = std::lower_bound(monoflags_.begin(), monoflags_.end(), c);
	return (pos != monoflags_.end()) && (*pos == c);
}
void dispatch::flags::clear() &
{
	flags_.clear();
	map_.clear();
	monoflags_.clear();
	plain_args_.clear();
}
void dispatch::flags::add_monoflag(char c)
{
	if (c == '\0')
	{
		return;
	}
	if (std::isspace(static_cast<unsigned char>(c)))
	{
		throw std::invalid_argument("whitespace monoflag");
	}
	auto ins_point = std::lower_bound(monoflags_.begin(), monoflags_.end(), c);
	if ((ins_point == monoflags_.end()) || (*ins_point != c))
	{
		monoflags_.insert(ins_point, c);
	}
}
