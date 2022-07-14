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

	std::ofstream os("dump");
	dump(os, obj);
}

void some(const std::string& mem)
{
	std::string_view sw_mem{ mem };
	{
		size_t p1 = 0;
		std::map<std::string, std::set<std::string>> cnt;
		while (true)
		{
			p1 = sw_mem.find("ItemId", p1);
			//p1 = sw_mem.find("ShipId", p1);
			if (p1 == std::string_view::npos)
				break;

			auto p2 = sw_mem.find(open_tag, p1);
			p2 = sw_mem.find(crlf_tag, p2);
			//std::string_view tmp2 = sw_mem.substr(p1, p2 - p1);
			p2 += 2;


			auto p3 = sw_mem.find(close_tag, p2);
			p3 += close_tag.size();
			std::string_view tmp = sw_mem.substr(p2, p3 - p2);
			{
				std::set<std::string> set_keys;
				std::string_view val;
				size_t i1 = 0;
				do {
					i1 = tmp.find('=', i1);
					if (i1 == std::string_view::npos)
						break;
					int beg = i1;
					beg--;

					while (true)
					{
						if (isalnum(tmp[beg])) beg--;
						else
						{
							beg++;
							break;
						}
					}
					std::string_view key = tmp.substr(beg, i1 - beg);
					if (key == "IType")
					{
						auto val_beg_pos = i1 + 1;
						auto val_end_pos = tmp.find(crlf_tag, i1);
						val = tmp.substr(val_beg_pos, val_end_pos - val_beg_pos);
					}
					set_keys.emplace(key.cbegin(), key.cend());
					i1 = tmp.find(crlf_tag, i1);
				} while (i1 != std::string_view::npos);

				cnt[{val.cbegin(), val.cend()}] = std::move(set_keys);
			}
			p1 = p3;
		}
		dump_to_mem(cnt);
		return;
	}
	//while()
}