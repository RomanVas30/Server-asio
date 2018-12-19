// Copyright 2018 Roman Vasyutin romanvas3008@gmail.com

#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

//using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::asio;

class talk_to_client {
public:
  talk_to_client(){}
  std::string username() const { return username_; }
  void answer_to_client(){
    try{
      read_request();
      process_request();
    }
    catch ( boost::system::system_error&){
      stop();
    }
    if ( timed_out())
      stop();
    }
  void set_clients_changed() { clients_changed_ = true; }
  ip::tcp::socket & sock() { return sock_; }
  bool timed_out() const{
    ptime now = microsec_clock::local_time();
    long long ms = (now - last_ping).total_milliseconds();
    return ms > 5000 ;
  }
  void stop(){
    boost::system::error_code err; sock_.close(err);
  }
  void read_request(){
    if ( sock_.available())
      already_read_ += sock_.read_some(
      buffer(buff_ + already_read_, max_msg - already_read_));
  }
  void process_request()
{
	bool found_enter = std::find(buff_, buff_ + already_read_, '\n') < buff_ + already_read_;
	if ( !found_enter)
		return; // message is not full
	// process the msg
	last_ping = microsec_clock::local_time();
	size_t pos = std::find(buff_, buff_ + already_read_, '\n') - buff_;
	std::string msg(buff_, pos);
	std::copy(buff_ + already_read_, buff_ + max_msg, buff_);
	already_read_ -= pos + 1;
	if ( msg.find("login ") == 0) on_login(msg);
	else if ( msg.find("ping") == 0) on_ping();
	else if ( msg.find("ask_clients") == 0) on_clients();
	else std::cerr << "invalid msg " << msg << std::endl;
}
void on_login(const std::string & msg)
{
	std::istringstream in(msg);
	in >> username_ >> username_;
	write("login ok\n");
	update_clients_changed();
}
void on_ping()
{
	write(clients_changed_ ? "ping client_list_changed\n" : "ping ok\n");
	clients_changed_ = false;
}
void on_clients()
{
	std::string msg;
	{
		boost::recursive_mutex::scoped_lock lk(cs);
		for( array::const_iterator b = clients.begin(), e = clients.end() ;b != e; ++b)
			msg += (*b)->username() + " ";
	}
	write("clients " + msg + "\n");
}
void write(const std::string & msg) { sock_.write_some(buffer(msg)); }

private:
	io_service service;
	std::vector<std::shared_ptr<talk_to_client>> clients;
	boost::recursive_mutex mutex;
	ip::tcp::socket sock_;
	enum { max_msg = 1024 };
	int already_read_;
	char buff_[max_msg];
	bool started_;
	std::string username_;
	bool clients_changed_;
	ptime last_ping;
};

#endif // INCLUDE_SERVER_HPP_
