#include "filefinder.hpp"
#include "programargs.hpp"

#include <set>
#include <map>
#include <iostream>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>

void filefinder::find()
{
	using namespace boost::filesystem;
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
		boost::system::error_code ec;
	
		directory_entry d(p);//, ec);
		d.status(ec);
		if (ec)	d.assign(current_path());//, ec);

		directory_iterator di(d, ec);
		if (ec) throw filesystem_error("Message", p, ec);

		std::map<std::time_t, directory_entry> m;
		std::set<std::time_t> for_sort_ft;
	
		for (auto f: di)
		{
			path fp(f);
			auto ext = fp.extension();
			if(ext.generic_string() == ".txt")
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
