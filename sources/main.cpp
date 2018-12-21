#include <server.hpp>

int main()
{
    auto talk = boost::make_shared<talk_to_client>();
    /*boost::thread_group threads;
    threads.create_thread(&talk_to_client::accept_thread, talk);
    threads.create_thread(&talk_to_client::handle_clients_thread, talk);
    threads.join_all();*/
    std::thread{&talk_to_client::accept_thread, talk}.join();
    std::thread{&talk_to_client::handle_clients_thread, talk}.join();
}
