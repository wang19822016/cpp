//
// Created by osx on 17/2/10.
//

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>


#include "AsioBase.h"

// https://github.com/mmoaay/boost-asio-cpp-network-programming-in-chinese

using boost::shared_ptr;

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::ip::address;


void AsioBase::test()
{

    io_service service;
    tcp::endpoint ep(address::from_string("www.baidu.com"), 80);
    tcp::socket sock(service);
    sock.connect(ep);
}

void AsioBase::server()
{
    using socket_ptr = shared_ptr<tcp::socket>;

    io_service service;
    tcp::endpoint ep(address::from_string("127.0.0.1"), 2000);
    tcp::acceptor acc(service, ep);

    while (true)
    {
        socket_ptr sock(new tcp::socket(service));
        acc.accept(*sock);

        while (true)
        {
            char data[512];
            size_t len = sock->read_some(boost::asio::buffer(data, 512));
            if (len > 0)
                write(*sock, boost::asio::buffer("ok", 2));
            sock->close();
            break;
        }
    }
}