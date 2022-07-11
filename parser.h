#pragma once
#include "Entities.h"

#include <string>
#include <string_view>
#include <stack>
#include <tuple>

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
		void * p;
		Curstruct st;
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
	}

	void parse(const std::string& mem);

private:
	void parse_line();

	void handle(Entities::Global* p);
	void handle(Entities::Player* p);
	void handle(Entities::EqList* p);
	void handle(Entities::StarList* p);
	void handle(Entities::Item*   p);
	void handle(void*             p);

	Parser_Ctx ctx;
};