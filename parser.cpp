#include "parser.h"

#include "Entities.h"
#include "convert.h"

#include <io.h>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <map>

static const std::string open_tag  = " ^{";
static const std::string close_tag = "}";

bool read_file(std::string& out, const std::wstring& path)
{
	FILE* f = _wfopen(path.c_str(), L"rb");
	if (!f) return false;
	long len = _filelength(_fileno(f));

	std::vector<char> buf;
	buf.resize(len);
	fread(buf.data(), len, 1, f);
	fclose(f);
	out.assign(buf.begin(), buf.end());
	return !out.empty();
}

bool getline(std::string::const_iterator beg,
			std::string::const_iterator end,
			std::string::const_iterator& line_beg,
			std::string::const_iterator& line_end,
			std::string::const_iterator& next_beg)
{
	if (beg != end)
	{
		const auto f = std::find(beg, end, '\r');
		if (f != end)
		{
			line_beg = beg;
			line_end = f;
			next_beg = f;
			if(f[1] == '\n') 
				std::advance(next_beg, 2);
			else
				std::advance(next_beg, 1);
			return true;
		}
		return false;
	}
	return false;
}

void trim_tabs(std::string::const_iterator& beg,
	std::string::const_iterator& end)
{
	while (beg !=end && *beg =='\t') ++beg;
	while (beg !=end && *(end - 1) == '\t') --end;
}

void parse(const std::string& mem)
{
	size_t lines = std::count(mem.begin(), mem.end(), '\n');
	size_t cnt_open = std::count(mem.begin(), mem.end(), '{');
	size_t cnt_close = std::count(mem.begin(), mem.end(), '}');

	if(cnt_open!= cnt_close)
		throw std::logic_error("mismatch {}");

	Parser p;
	p.parse(mem);
	return;
}

void Parser::parse(const std::string& mem)
{
	ctx.init(mem);

	std::string::const_iterator next_beg = ctx.main_begin_;

	std::string::const_iterator line_beg, line_end;
	while (getline( next_beg, ctx.main_end_,
					line_beg, line_end,
					next_beg ))
	{
		trim_tabs(line_beg, line_end);

		ctx.set_line(line_beg, line_end);
		parse_line();
	}
}

void Parser::parse_line()
{
	// resolve current struct
	// save(push) it to ctx
	// Идея в том, чтоб не реккурсивно идти - один проход одно заполнение
	// чего из контекста выбирать, диспатчить по нему,
	// в хендлерах для конкретной структуры заполнять поля
	// при встрече новой структуры менять контекст (+ создание) и возвращаться
	std::string out;
	out.assign(ctx.line_beg_, ctx.line_end_);

	//Entities::unpack_goods_str("1,22,333,4444,0,55555");

	auto& s = ctx.stack.top();
	std::visit(Handler{ctx}, s.p);
#if 0
	switch (s.st)
	{
		case Curstruct::Global:
			handle((Entities::Global*)s.p);
			break;
		case Curstruct::Player:
			handle((Entities::Player*)s.p);
			break;
		case Curstruct::EqList:
			handle((Entities::EqList*)s.p);
			break;		
		case Curstruct::Item:
			handle((Entities::Item*)s.p);
			break;
		case Curstruct::StarList:
			handle((Entities::StarList*)s.p);
			break;
		case Curstruct::Unknown:
			handle((void*)s.p);
		default:
			break;
	}
#endif
	return;
}

#if 0
void Parser::handle(Entities::Global* p)
{
	//int IDay;
	//Player* Player;
	//StarList StarList;

	
}

void Parser::handle(Entities::Player* p)
{
	handle((void*)p);
}

void Parser::handle(Entities::EqList* p)
{
	handle((void*)p);
}

void Parser::handle(Entities::StarList * p)
{
	handle((void*)p);
}

void Parser::handle(Entities::Item* p)
{
	handle((void*)p);
}

void Parser::handle(void* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		ctx.stack.push({ nullptr, Curstruct::Unknown });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		auto[key, value] = ctx.get_kv();
		return;
	}
}
#endif //0

void Parser_Ctx::init(const std::string& mem)
{
	main_begin_ = cbegin(mem);
	main_end_   = cend(mem);

	out         = new Entities::Global{};
	stack.push({out});
}

void Parser_Ctx::set_line( std::string::const_iterator line_beg,
						   std::string::const_iterator line_end )
{
	line_beg_ = std::move(line_beg);
	line_end_ = std::move(line_end);
}

bool Parser_Ctx::is_object_open() const
{
	return std::search(line_beg_, line_end_, open_tag.cbegin(), open_tag.cend()) != line_end_;
}

bool Parser_Ctx::is_object_close() const
{
	return std::search(line_beg_, line_end_, close_tag.cbegin(), close_tag.cend()) != line_end_;
}

std::string_view 
Parser_Ctx::get_object_name() const
{
	const auto f = std::search(line_beg_, line_end_, open_tag.cbegin(), open_tag.cend());
	if (f != line_end_)
	{
		return std::string_view(&*line_beg_, std::distance(line_beg_, f));
	}
	return {};
}

std::string_view
Parser_Ctx::get_cur_line_str() const
{
	return std::string_view( &*line_beg_, std::distance(line_beg_, line_end_) );
}

std::pair<std::string_view, std::string_view>
Parser_Ctx::get_kv() const
{
	const auto f = std::find(line_beg_, line_end_, '=');
	if (f != line_end_)
	{
		auto value_beg = f + 1;
		return 
		{ 
			std::string_view(&*line_beg_, std::distance(line_beg_, f)),
			std::string_view(&*value_beg, std::distance(value_beg, line_end_))
		};
	}
	return {};
}

// unknown type correct handle
void Handler::operator()(Entities::Unknown*)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		ctx.stack.push({ (Entities::Unknown*)nullptr });
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
}

void Handler::operator()(Entities::Global* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		if (obj_name == "Player")
		{
			p->Player = new Entities::Player{};
			ctx.stack.push({p->Player});
		}
		else if (obj_name == "StarList")
		{
			ctx.stack.push({ &p->StarList });
		}
		else
		{
			ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		const auto[key, value] = ctx.get_kv();

		if (key == "IDay")
			p->IDay = conv::to_int(value);
	}
}

void Handler::operator()(Entities::Player* p)
{
	if (ctx.is_object_open())
	{
		auto obj_name = ctx.get_object_name();
		
		{
			ctx.stack.push({ (Entities::Unknown*)nullptr });
		}
	}
	else if (ctx.is_object_close())
	{
		ctx.stack.pop();
	}
	else
	{
		const auto[key, value] = ctx.get_kv();

		if (     key == "ICurStarId")
			p->ICurStarId = conv::to_int(value);
		else if (key == "IFullName")
			p->IFullName = value;
		else if (key == "IType")
			p->IType = Entities::str2type<Entities::Type>(value);
		else if (key == "Name")
			p->Name = value;
		else if (key == "IPlanet")
			p->IPlanet = value;
		else if (key == "Goods")
			p->Goods = Entities::unpack_goods_str(value);

	}
}

void Handler::operator()(Entities::StarList* p)
{

}

void Handler::operator()(Entities::Star* p)
{

}