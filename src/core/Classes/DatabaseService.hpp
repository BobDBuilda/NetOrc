#ifndef DATABASE_SERVICE_HPP
#define DATABASE_SERVICE_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <vector>

class IDatabaseService {
public:
    virtual ~IDatabaseService() = default;
    virtual void saveFlow(uint64_t dpid, uint32_t in_port, int action_port) = 0;
};

class DatabaseService : public IDatabaseService {
private:
    sqlite3* db;

public:
    DatabaseService(const std::string& db_name = "netorc.db") {
        if (sqlite3_open(db_name.c_str(), &db)) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        } else {
            std::cout << "Opened database successfully" << std::endl;
            createTables();
        }
    }

    ~DatabaseService() {
        sqlite3_close(db);
    }

    void createTables() {
        const char* sql = "CREATE TABLE IF NOT EXISTS flows ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "dpid INTEGER,"
                          "in_port INTEGER,"
                          "dl_src TEXT,"
                          "dl_dst TEXT,"
                          "nw_src TEXT,"
                          "nw_dst TEXT,"
                          "tp_src INTEGER,"
                          "tp_dst INTEGER,"
                          "action_port INTEGER"
                          ");";
        
        char* errMsg = 0;
        if (sqlite3_exec(db, sql, nullptr, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    void saveFlow(uint64_t dpid, uint32_t in_port, int action_port) override {
        std::string sql = "INSERT INTO flows (dpid, in_port, action_port) VALUES (" +
                          std::to_string(dpid) + ", " +
                          std::to_string(in_port) + ", " +
                          std::to_string(action_port) + ");";

        char* errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL insert error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "[Database] Flow persisted for DPID: " << dpid << std::endl;
        }
    }
};

#endif // DATABASE_SERVICE_HPP
