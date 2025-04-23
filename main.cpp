#include <iostream>
#include <libpq-fe.h>

// sudo -u postgres psql
// # In the psql prompt:
// CREATE USER yifan WITH PASSWORD '123';
// CREATE DATABASE testdb OWNER yifan;
// \q
int main() {
    // 1) 建立连接
    const char* conninfo = "host=localhost port=5432 dbname=testdb user=yifan password=123";
    // or use environment variables:
    // const char* conninfo = getenv("DATABASE_URL");
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn);
        PQfinish(conn);
        return 1;
    }
    std::cout << "Connected successfully\n";

    // 2) 执行查询
    PGresult* res = PQexec(conn, "SELECT version()");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT failed: " << PQerrorMessage(conn);
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // 3) 读取结果
    int nrows = PQntuples(res);
    for (int i = 0; i < nrows; i++) {
        std::cout << "PostgreSQL version: "
                  << PQgetvalue(res, i, 0)
                  << "\n";
    }

    // 4) 清理
    PQclear(res);
    PQfinish(conn);
    return 0;
}
