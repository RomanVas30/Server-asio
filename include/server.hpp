// Copyright 2018 Roman Vasyutin romanvas3008@gmail.com

#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <algorithm>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace boost::asio;
using namespace boost::posix_time;

class talk_to_client {
public:
    talk_to_client() : sock_(service) {}
    std::string username() const {
        return username_;
    }
    void answer_to_client();
    void set_clients_changed();
    void stop();
    bool timed_out() const;
    ip::tcp::socket& sock();
    void read_request();
    void process_request();
    void on_login(const std::string & msg);
    void on_ping();
    void on_clients();
    void write(const std::string & msg);
    void accept_thread();
    void handle_clients_thread();

private:
    io_service service;
    std::vector<boost::shared_ptr<talk_to_client>> clients;
    boost::recursive_mutex mutex;
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char buff_[max_msg];
    bool started_;
    std::string username_;
    bool clients_changed_;
    ptime last_ping;
};

#endif // INCLUDE_SERVER_HPP_
