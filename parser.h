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
			Entities::Treasure*
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

	void operator()(Entities::Unknown*);

	// void operator()(Entities::Global* p);
	void on_new_obj(Entities::Global* p, std::string_view obj_name);
	void on_kv     (Entities::Global* p, std::string_view key, std::string_view value);

	//void operator()(Entities::Player* p);
	void on_new_obj(Entities::Player* p, std::string_view obj_name);
	void on_kv(Entities::Player*      p, std::string_view key, std::string_view value);

	//void operator()(Entities::StarList* p);
	void on_new_obj(Entities::StarList* p, std::string_view obj_name);

	//void operator()(Entities::Star* p);
	void on_new_obj(Entities::Star* p, std::string_view obj_name);
	void on_kv(Entities::Star*      p, std::string_view key, std::string_view value);

	//void operator()(Entities::EqList * p);
	void on_new_obj(Entities::EqList* p, std::string_view obj_name);

	//void operator()(Entities::ShipList * p);
	void on_new_obj(Entities::ShipList* p, std::string_view obj_name);

	void operator()(Entities::Ship * p);
	void operator()(Entities::PlanetList * p);
	void operator()(Entities::Planet * p);
	void operator()(Entities::Junk * p);
	void operator()(Entities::EqShop * p);
	void operator()(Entities::Treasure * p);

	void operator()(Entities::Item * p);

	//defaults
	template <typename T>
	void on_kv(T* p, std::string_view key, std::string_view value)
	{
	}

	//defaults
	template <typename T>
	void on_new_obj(T* p, std::string_view obj_name)
	{
		ctx.stack.push({ (Entities::Unknown*)nullptr });
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
	void default_impl(void* p);

	Parser_Ctx& ctx;
};