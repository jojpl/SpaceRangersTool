#include "parser.h"

#include <set>
#include <map>
#include <ostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <string_view>


static const std::string open_tag = " ^{";
static const std::string close_tag = "}";
static const std::string crlf_tag = "\r\n";

void dump(std::ostream& os, std::set<std::string>& set, size_t tabs_num)
{
	std::string tabs{ "\t", tabs_num };
	for (auto& val : set)
	{
		os <<
			tabs << "<" << val << "/>"
			<< std::endl;
	}
}

void dump(std::ostream& os, std::map<std::string, std::set<std::string>>& obj)
{
	std::string op{ "<" }, cl{ ">" },
		op_cl{ "</" }, cl_cl{ "/>" };
	for (auto&[IType, set] : obj)
	{
		os <<
			"<" << IType << ">"
			<< std::endl;

		dump(os, set, 1);


		os <<
			"</" << IType << ">"
			<< std::endl;

		os << std::endl;
	}
}

void dump_to_mem(std::map<std::string, std::set<std::string>>& obj)
{
	std::stringstream ss;
	dump(ss, obj);
}

void dump_to_file(std::map<std::string, std::set<std::string>>& obj)
{

	std::ofstream os("dump_2");
	dump(os, obj);
}

void some(const std::string& mem)
{
	std::string_view sw_mem{ mem };
	std::map<std::string, std::set<std::string>> cnt;
	
	Parser_Ctx ctx;

	auto is_item_obj = [](Parser_Ctx& ctx)
	{
		if(!ctx.stack.empty())
		{
			if(std::get_if<Entities::Item*>(&ctx.stack.top()))
				return true;
		}
		return false;
	};

	std::string typeId;
	std::set<std::string> s;

	std::string_view line, next_line{ mem };
	while (getline(next_line, line, next_line))
	{
		trim_tabs(line);

		ctx.line_ = line;

		// parse
		if (ctx.is_object_open())
		{
			auto obj = ctx.get_object_name();
			if (obj.find("ItemId") == 0) //finded in beg str
			{
				ctx.stack.push((Entities::Item*)0);
			}
			else
				ctx.stack.push((Entities::Unknown*)0);
		}
		else if (ctx.is_object_close())
		{
			if(is_item_obj(ctx))
			{
				cnt[typeId] = s;
				s.clear();
			}
			ctx.stack.pop();
		}
		else if(is_item_obj(ctx))
		{
			const auto [key, value] = ctx.get_kv();
			if (key == "IType")
			{
				std::string_view t = value;
				if (t.find("Art") == 0)
				{
					t = value.substr(0, 3);
				}
				else if(t.size() == 3 && t.find("W") == 0)
				{
					t = value.substr(0, 1);
				}
				else if(model::converter<Entities::GoodsEnum>().is_string_for(t))
				{
					t = "goods_arr";
				}
				typeId = t;
			}
			else
			{
				s.insert(std::string{ key });
			}
		}
	}

	dump_to_file(cnt);
}

