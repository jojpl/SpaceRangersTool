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


struct Parser_Ctx
{
	void init(const std::string& mem);

	void set_line(	std::string_view	);

	std::string_view line_;
	
	using variants = std::variant<
			Entities::Unknown*, //unknown type
			Entities::Global*,
			Entities::Player*,
			//Entities::StarList*,
			Entities::EqList*,
			Entities::Item*
		>;

	bool is_object_open() const;
	bool is_object_close() const;
	std::string_view get_object_name() const;
	
	std::pair<std::string_view, std::string_view>
	get_kv() const;

	std::stack<variants> stack;

	Entities::Global * out;
};

class Parser
{
public:

	void parse(const std::string& mem);

private:
	void parse_line();
	Parser_Ctx ctx;
};

class Handler
{
public:
	Handler(Parser_Ctx& ctx_)
		:ctx(ctx_)
	{}

	void operator()(Entities::Unknown*);

	void operator()(Entities::Global* p);
	void operator()(Entities::Player* p);
	//void operator()(Entities::StarList* p);
	//void operator()(Entities::Star* p);
	void operator()(Entities::EqList * p);

	void operator()(Entities::Item * p);



private:

	Parser_Ctx& ctx;
};