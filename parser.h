#pragma once
#include "Entities.h"
#include "convert.h"

#include <string>
#include <string_view>
#include <stack>
#include <tuple>

//#include "boost\variant.hpp"
#include <variant>

bool read_file(std::string& out, const std::wstring& path);
void parse(const std::string& mem);
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

			Entities::Global*,
			Entities::Player*,
			Entities::Warrior*,
			Entities::StarList*,
			Entities::Star*,
			Entities::EqList*,
			//Entities::ArtsList*,
			Entities::Item*,
			Entities::ShipList*,
			Entities::Ship*,
			Entities::PlanetList*,
			Entities::Planet*,
			Entities::Junk*,
			Entities::EqShop*,
			Entities::Treasure*,
			Entities::HiddenItem*
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

};

class Parser
{
public:
	void parse(const std::string& mem);

private:
	void init_ctx(std::string_view mem);

	void parse_line();

	Parser_Ctx ctx;
	Entities::Global * out;
};

class Handler
{
public:
	Handler(Parser_Ctx& ctx_)
		:ctx(ctx_)
	{}

	// Global
	void on_new_obj(Entities::Global* p, std::string_view obj_name);
	void on_kv     (Entities::Global* p, std::string_view key, std::string_view value);

	// Player
	void on_new_obj(Entities::Player* p, std::string_view obj_name);
	void on_kv(Entities::Player*      p, std::string_view key, std::string_view value);

	// StarList
	void on_new_obj(Entities::StarList* p, std::string_view obj_name);

	// Star
	void on_new_obj(Entities::Star* p, std::string_view obj_name);
	void on_kv(Entities::Star*      p, std::string_view key, std::string_view value);

	// EqList
	void on_new_obj(Entities::EqList* p, std::string_view obj_name);

	// ShipList
	void on_new_obj(Entities::ShipList* p, std::string_view obj_name);

	// Ship
	void on_new_obj(Entities::Ship* p, std::string_view obj_name);
	void on_kv(Entities::Ship*      p, std::string_view key, std::string_view value);

	// PlanetList
	void on_new_obj(Entities::PlanetList* p, std::string_view obj_name);

	// Planet
	void on_new_obj(Entities::Planet* p, std::string_view obj_name);
	void on_kv(Entities::Planet*      p, std::string_view key, std::string_view value);

	// Junk
	void on_new_obj(Entities::Junk* p, std::string_view obj_name);

	// EqShop
	void on_new_obj(Entities::EqShop* p, std::string_view obj_name);

	// Treasure
	void on_new_obj(Entities::Treasure* p, std::string_view obj_name);

	// HiddenItem
	void on_new_obj(Entities::HiddenItem* p, std::string_view obj_name);
	void on_kv(Entities::HiddenItem*      p, std::string_view key, std::string_view value);

	// Item
	void on_kv(Entities::Item*      p, std::string_view key, std::string_view value);

	//defaults
	template <typename T>
	void on_kv(T* p, std::string_view key, std::string_view value)
	{
	}

	//defaults
	template <typename T>
	void on_new_obj(T* p, std::string_view obj_name)
	{
	}

	template <typename T>
	void operator()(T* t)
	{
		if (ctx.is_object_open())
		{
			auto obj_name = ctx.get_object_name();

			auto sz = ctx.stack.size();
			on_new_obj(t, obj_name);
			auto new_sz = ctx.stack.size();

			if (sz == new_sz)
				ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
		else if (ctx.is_object_close())
		{
			ctx.stack.pop();
		}
		else if (ctx.is_object_kv())
		{
			const auto[key, value] = ctx.get_kv();
			on_kv(t, key, value);
		}
	}

private:

	Parser_Ctx& ctx;
};