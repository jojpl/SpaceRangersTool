#pragma once
#include "model.hpp"

#include <string>
#include <string_view>
#include <stack>
#include <set>
#include <tuple>

#include <variant>
namespace parser
{

using namespace Entities;

Global* parse(const std::string& mem);

void trim_tabs(std::string_view& beg);

bool getline(std::string_view str,
	std::string_view& out_line,
	std::string_view& out_next);

std::pair<std::string_view, std::string_view>
	split_to_kv(std::string_view line);

struct Parser_Ctx
{
	using variants = std::variant<
			Entities::Unknown*, //unknown type

			Global*,
			Player*,
			Warrior*,
			StarList*,
			Star*,
			EqList*,
			ArtsList*,
			Item*,
			ShipList*,
			Ship*,
			Station*,
			PlanetList*,
			Planet*,
			Junk*,
			EqShop*,
			Treasure*,
			HiddenItem*,
			HoleList*,
			Hole*,
			Garrison*
		>;

	bool getline();

	bool is_object_open() const;

	bool is_object_close() const;

	bool is_object_kv() const;

	std::string_view get_object_name() const;
	
	std::pair<std::string_view, std::string_view>
	get_kv() const;

	std::string_view line_;

	std::string_view tail_;

	std::stack<variants> stack;

	Location location_;

	std::set<std::string_view> unregistered_obj;

};

class Parser
{
public:
	void parse(const std::string& mem);

	Global *get_parsed();

private:
	void init_ctx(std::string_view mem);

	void fix_skiped_look_forwarded_options();

	void parse_line();

	Parser_Ctx ctx;
	Global * out_ = nullptr;
};

class Handler
{
public:
	Handler(Parser_Ctx& ctx_)
		:ctx(ctx_)
	{}

	// main common algo
	template <typename T>
	void operator()(T* t)
	{
		if (ctx.is_object_open())
		{
			auto obj_name = ctx.get_object_name();
			if (!on_new_obj(t, obj_name))
			{
				//std::string nn = { obj_name.data(), obj_name.size()};
				//auto c = nn.c_str();
				//ctx.unregistered_obj.insert(obj_name);
				ctx.stack.push({ (Entities::Unknown*)nullptr });
			}
		}
		else if (ctx.is_object_close())
		{
			on_close_obj(t);
			if(ctx.stack.empty()) throw std::logic_error("wrong format");
			ctx.stack.pop();
		}
		else if (ctx.is_object_kv())
		{
			auto[key, value] = ctx.get_kv();
			on_kv(t, key, value);
		}
	}

private:
	// Global
	bool on_new_obj(Global* p, std::string_view obj_name);
	bool on_kv     (Global* p, std::string_view key, std::string_view value);

	// Player
	bool on_new_obj(Player* p, std::string_view obj_name);
	bool on_kv(Player*      p, std::string_view key, std::string_view value);

	// StarList
	bool on_new_obj(StarList* p, std::string_view obj_name);

	// Star
	bool on_new_obj(Star* p, std::string_view obj_name);
	bool on_kv(Star*      p, std::string_view key, std::string_view value);
	void on_close_obj(Star* p);

	// EqList
	bool on_new_obj(EqList* p, std::string_view obj_name);

	// ArtList
	bool on_new_obj(ArtsList* p, std::string_view obj_name);

	// ShipList
	bool on_new_obj(ShipList* p, std::string_view obj_name);

	// Ship
	bool on_new_obj(Ship* p, std::string_view obj_name);
	bool on_kv(Ship*      p, std::string_view key, std::string_view value);

	// Station
	bool on_new_obj(Station* p, std::string_view obj_name);
	bool on_kv(Station*      p, std::string_view key, std::string_view value);
	void on_close_obj(Station* p);

	// PlanetList
	bool on_new_obj(PlanetList* p, std::string_view obj_name);

	// Planet
	bool on_new_obj(Planet* p, std::string_view obj_name);
	bool on_kv(Planet*      p, std::string_view key, std::string_view value);
	void on_close_obj(Planet* p);

	// Junk
	bool on_new_obj(Junk* p, std::string_view obj_name);

	// EqShop
	bool on_new_obj(EqShop* p, std::string_view obj_name);

	// Treasure
	bool on_new_obj(Treasure* p, std::string_view obj_name);

	// HiddenItem
	bool on_new_obj(HiddenItem* p, std::string_view obj_name);
	bool on_kv(HiddenItem*      p, std::string_view key, std::string_view value);

	// Item
	bool on_kv(Item*      p, std::string_view key, std::string_view value);

	// HoleList
	bool on_new_obj(HoleList* p, std::string_view obj_name);
	// Hole
	bool on_kv(Hole*      p, std::string_view key, std::string_view value);

	// Garrison
	bool on_new_obj(Garrison* p, std::string_view obj_name);

	// Warrior
	bool on_new_obj(Warrior* p, std::string_view obj_name);
	bool on_kv(Warrior*      p, std::string_view key, std::string_view value);

	//defaults
	template <typename T>
	bool on_kv(T* p, std::string_view key, std::string_view value)
	{
		return false;
	}

	//defaults
	template <typename T>
	bool on_new_obj(T* p, std::string_view obj_name)
	{
		return false;
	}

	//defaults
	template <typename T>
	void on_close_obj(T* p)
	{
	}

	Parser_Ctx& ctx;
};

} // namespace parser