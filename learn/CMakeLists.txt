# cmake最低版本
cmake_minimum_required(VERSION 3.6)

# 项目名称
project(learn)

#
set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -framework GLUT -framework OpenGL")

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}
        /usr/local/include
        ${PROJECT_SOURCE_DIR}/opengl)
        #${PROJECT_SOURCE_DIR}/mysql)

# 库文件目录
link_directories(/usr/local/lib /usr/local/Cellar/glew/2.0.0/lib /usr/local/Cellar/mysql-connector-c++/1.1.8/lib)

# 包含源码的子目录
add_subdirectory(opengl)
#add_subdirectory(mysql)

# 当前目录源码
aux_source_directory(. DIR_SRCS)

# 生成目标文件
add_executable(learn ${DIR_SRCS})

# 链接库
target_link_libraries(learn opengl GLEW.2.0 glfw3) #mysql mysqlcppconn)

