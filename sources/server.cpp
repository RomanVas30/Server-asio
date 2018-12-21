// Copyright 2018 Roman Vasyutin romanvas3008@gmail.com

#include <server.hpp>

void talk_to_client::answer_to_client() {
    try {
        read_request();
        process_request();
    }
    catch ( boost::system::system_error&) {
        stop();
    }
    if ( timed_out())
        stop();
}

void talk_to_client::set_clients_changed() {
    clients_changed_ = true;
}

ip::tcp::socket& talk_to_client::sock() {
    return sock_;
}

bool talk_to_client::timed_out() const {
    ptime now = microsec_clock::local_time();
    long long ms = (now - last_ping).total_milliseconds();
    return ms > 5000 ;
}

void talk_to_client::stop() {
    boost::system::error_code err;
    sock_.close(err);
}

void talk_to_client::read_request() {
    if ( sock_.available())
        sock_.read_some(buffer(buff_, max_msg));
}

void talk_to_client::process_request(){
    bool found_enter = std::find(buff_, buff_ + max_msg, '\n') < buff_ + max_msg;
    if ( !found_enter)
        return;
    last_ping = microsec_clock::local_time();
    //size_t pos = std::find(buff_, buff_ + max_msg, '\n') - buff_;
    std::string msg(buff_);
    //std::copy(buff_ + already_read_, buff_ + max_msg, buff_);
    //already_read_ -= pos + 1;
    if ( msg.find("login ") == 0) on_login(msg);
    else if ( msg.find("ping") == 0) on_ping();
    else if ( msg.find("ask_clients") == 0) on_clients();
    else std::cerr << "invalid msg " << msg << std::endl;
}

void talk_to_client::on_login(const std::string & msg){
    std::istringstream in(msg);
    in >> username_ >> username_;
    write("login ok\n");
    set_clients_changed();
}

void talk_to_client::on_ping(){
    write(clients_changed_ ? "ping client_list_changed\n" : "ping ok\n");
    clients_changed_ = false;
}
void talk_to_client::on_clients(){
    std::string msg;
    {
        boost::recursive_mutex::scoped_lock lk(mutex);
        for( std::vector<boost::shared_ptr<talk_to_client>>::const_iterator b = clients.begin(), e = clients.end() ; b != e; ++b)
            msg += (*b)->username() + " ";
    }
    write("clients " + msg + "\n");
}

void talk_to_client::write(const std::string & msg) {
    sock_.write_some(buffer(msg));
}

void talk_to_client::accept_thread(){
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
    while ( true)
    {
        boost::shared_ptr<talk_to_client> new_( new talk_to_client);
        acceptor.accept(new_->sock());
        boost::recursive_mutex::scoped_lock lk(mutex);
        clients.push_back(new_);
    }
}

void talk_to_client::handle_clients_thread(){
    while ( true)
    {
        boost::this_thread::sleep( millisec(1));
        boost::recursive_mutex::scoped_lock lk(mutex);
        for(std::vector<boost::shared_ptr<talk_to_client>>::iterator b = clients.begin(),e = clients.end(); b != e; ++b)
            (*b)->answer_to_client();
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     boost::bind(&talk_to_client::timed_out, _1)), clients.end());
    }
}

