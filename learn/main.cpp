#include <iostream>
#include "Chapter02.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

int main()
{
    /*
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
     */


    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    //glViewport(0, 0, width, height);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}