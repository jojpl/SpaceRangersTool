#pragma once
#include "Entities.h"

#include <string>
#include <string_view>
#include <stack>
#include <tuple>

//#include "boost\variant.hpp"
#include <variant>

bool read_file(std::string& out, const std::wstring& path);
void parse(const std::string& mem);

enum class Curstruct;

struct Parser_Ctx
{
	void init(const std::string& mem);

	void set_line(	std::string::const_iterator line_beg,
					std::string::const_iterator line_end);

	std::string::const_iterator main_begin_,
								main_end_;

	std::string::const_iterator line_beg_,
								line_end_;
	

	struct Stack
	{
		std::variant<
			Entities::Unknown*, //unknown type
			Entities::Global*,
			Entities::Player*,
			//Entities::StarList*,
			Entities::EqList*,
			Entities::Item*
		> p;
	};

	bool is_object_open() const;
	bool is_object_close() const;
	std::string_view get_object_name() const;
	std::string_view get_cur_line_str() const;
	
	std::pair<std::string_view, std::string_view>
	get_kv() const;

	std::stack<Stack> stack;

	Entities::Global * out;
};

class Parser
{
public:
	Parser()
	{
		Entities::kv::init_storage();
	}

	void parse(const std::string& mem);

private:
	void parse_line();

#if 0
	void handle(Entities::Global* p);
	void handle(Entities::Player* p);
	void handle(Entities::EqList* p);
	void handle(Entities::StarList* p);
	void handle(Entities::Item*   p);
	void handle(void*             p);
#endif //0
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