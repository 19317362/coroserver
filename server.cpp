//
//  server.cpp
//  coroserver
//
//  Created by Windoze on 13-2-24.
//  Copyright (c) 2013 0d0a.com. All rights reserved.
//

#include <functional>
#include <memory>
#include <thread>
#include <boost/bind.hpp>
#include <boost/asio/spawn.hpp>
#include "server.h"

server::server(std::function<bool(async_tcp_stream&)> protocol_processor,
               const std::string &address,
               const std::string &port,
               std::size_t thread_pool_size)
: protocol_processor_(protocol_processor)
, thread_pool_size_(thread_pool_size)
, io_service_()
, signals_(io_service_)
, acceptor_(io_service_)
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(std::bind(&server::close, this));
    
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    
    open();
}

void server::run() {
    // Create a pool of threads to run all of the io_services.
    std::vector<std::shared_ptr<std::thread> > threads;
    for (std::size_t i=0; i<thread_pool_size_; ++i) {
        // NOTE: It's weird that std::bind doesn't compile here
        threads.push_back(std::move(std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run,
                                                                              &io_service_))));
    }
    
    // Wait for all threads in the pool to exit.
    for (std::size_t i=0; i<threads.size(); ++i)
        threads[i]->join();
}

void server::open() {
    boost::asio::spawn(io_service_,
                       [&](boost::asio::yield_context yield) {
                           for (;;) {
                               boost::system::error_code ec;
                               boost::asio::ip::tcp::socket socket(io_service_);
                               acceptor_.async_accept(socket, yield[ec]);
                               if (!ec)
                                   handle_connect(std::move(socket));
                           }
                       });
}

void server::close()
{ io_service_.stop(); }

void server::handle_connect(boost::asio::ip::tcp::socket &&socket) {
    boost::asio::spawn(boost::asio::strand(io_service_),
                       [this, &socket](boost::asio::yield_context yield) {
                           async_tcp_stream s(std::move(socket), yield);
                           try {
                               protocol_processor_(s);
                           } catch (std::exception const& e) {
                               s << "Exception caught:" << e.what() << std::endl;
                           } catch(...) {
                               s << "Unknown exception caught" << std::endl;
                           }
                       });
}
