#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include "performance_tracker.hpp"

using namespace soci;

backend_factory const &backEnd = *soci::factory_sqlite3();

void Db_test()
{
	try
	{
		connection_parameters par(backEnd, ":memory:");
		session sql(par);

		//try { sql << "drop table test1"; }
		//catch (soci_error const &) {} // ignore if error

		sql <<
			"create table test1 ("
			"    id integer"
			//"    id integer,"
			//"    name varchar(100)"
			")";
		int id = 99;
		statement stmt(sql);
		stmt.alloc();
		stmt.prepare("insert into test1(id) values(:i)");
		stmt.exchange(use(id));
		stmt.define_and_bind();
		//int i;
		//statement st = (sql.prepare <<
		//	"insert into test1(id, name) values(:i, \'John\')", use(i));

		//performance_tracker tr(__FUNCTION__);
		for (id = 0; id < 1'000'000; id++)
		{
			stmt.execute(true);
			if(!(id%100'000)) std::cout << id << '\n';
		}
		//st.execute(true);
		//st.fetch();
		int cnt = 0;
		sql << "select COUNT(*) from test1", into(cnt);
		return;

		rowid rid(sql);
		sql << "select oid from test1 where id = 7", into(rid);

		//int id;
		std::string name;

		sql << "select id, name from test1 where oid = :rid",
			into(id), into(name), use(rid);

		sql << "drop table test1";
	}
	catch (const soci_error& ec)
	{
		auto gg = ec.what();
	}
	catch (const std::exception& ec)
	{
		auto gg = ec.what();
	}
	return;
}