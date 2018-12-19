#include <server.hpp>

void accept_thread()
{
ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
while ( true)
{
	client_ptr new_( new talk_to_client);
	acceptor.accept(new_->sock());
	boost::recursive_mutex::scoped_lock lk(cs);
	clients.push_back(new_);
}
}
void handle_clients_thread()
{
	while ( true)
	{
		boost::this_thread::sleep( millisec(1));
		boost::recursive_mutex::scoped_lock lk(cs);
		for(array::iterator b = clients.begin(),e = clients.end(); b != e; ++b)
		(*b)->answer_to_client();
		// erase clients that timed out
		clients.erase(std::remove_if(clients.begin(), clients.end(),
		boost::bind(&talk_to_client::timed_out,_1)), clients.end());
	}
}
int main(int argc, char* argv[])
{
	boost::thread_group threads;
	threads.create_thread(accept_thread);
	threads.create_thread(handle_clients_thread);
	threads.join_all();
}
