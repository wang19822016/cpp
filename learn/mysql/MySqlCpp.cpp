//
// Created by osx on 17/2/10.
//

#include "MySqlCpp.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
void MySqlCpp::test()
{

    using sql::Driver;
    using sql::Connection;
    using sql::PreparedStatement;

    try {
        Driver *driver = get_driver_instance();
        Connection *con = driver->connect("tcp://10.10.10.23", "root", "123456");
        con->setSchema("report");
        con->setAutoCommit(false);

        std::cout << time(nullptr) << "\n";
        PreparedStatement *prep_stmt = con->prepareStatement(
                "insert into 11_user_login(id, userId, serverTime, serverDate) values (?, ?, now(), '2017-02-09 12:00:00')");
        for (int i = 0; i < 2000; i++) {
            prep_stmt->setInt(1, i);
            prep_stmt->setInt(2, i);
            prep_stmt->executeUpdate();
        }
        con->commit();

        delete prep_stmt;
        delete con;

        std::cout << time(nullptr) << "\n";
    } catch (sql::SQLException &e) {
        std::cout << e.what() << "\n";
    }
}