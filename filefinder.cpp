#include "filefinder.hpp"
#include "programargs.hpp"

#include <filesystem>
#include <set>
#include <map>
#include <iostream>
#include <system_error>

void filefinder::find()
{
	using namespace std::filesystem;
	std::string file;

	try
	{
		path p;
		const auto& opt = options::get_opt();
		if(opt.dir)
			p = opt.dir.value();
		else
		{
#ifdef _WIN32
			const char* home_drive = std::getenv("HOMEDRIVE");
			const char* home_patch = std::getenv("HOMEPATH");
			if(home_drive && home_patch)
			{
				p/= home_drive;
				p/= home_patch;
				p/= "Documents\\spacerangershd\\save\\";
			}
#endif
		}
	
		std::error_code ec;
		directory_entry d(p, ec);
		if (ec)	d.assign(std::filesystem::current_path(), ec);

		directory_iterator di(d, ec);
		if (ec) throw filesystem_error("Message", p, ec);

		std::map<file_time_type, directory_entry> m;
		std::set<file_time_type> for_sort_ft;
	
		for (auto f: di)
		{
			path fp(f);
			auto ext = fp.extension();
			if(ext == L".txt")
			{
				auto ftime = last_write_time(f);
				m.emplace(ftime, f);
				for_sort_ft.emplace(ftime);
			}
		}

		if(for_sort_ft.empty())
			throw std::logic_error("dump .txt file not found!");

		auto last_ft = *for_sort_ft.rbegin();
		auto fresh_f = m.at(last_ft);
		file = fresh_f.path().generic_string();
	}
	catch (std::exception& ec)
	{
		std::cerr << ec.what();
		return;
	}
		
	handler(file);
}
