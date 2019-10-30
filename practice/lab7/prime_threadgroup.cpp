#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define NUM_THREAD_IN_POOL 10

boost::asio::io_service io_print;

void print(int seq, int start, int end, int cnt) {
	std::cout << '(' << seq << ")number of primes in " << start << " ~ " << end << " is " << cnt << '\n';
}

int calculate_prime(int start, int end) {
	int cnt = 0;
	bool is_prime = true;
	for (int n = start; n <= end; n++) {
		is_prime = true;
		for (int div = 2; div * div <= n; div++) {
			if (n % div == 0) {
				is_prime = false;
				break;
			}
		}
		if (is_prime) cnt++;
	}
	return cnt;
}

void calculate_prime_and_post_print(int seq, int start, int end) {
	int cnt = calculate_prime(start, end);
	io_print.post(boost::bind(&print, seq, start, end, cnt));
}

int main(void) {
    boost::asio::io_service io;
    boost::thread_group threadpool;
    boost::asio::io_service::work* work = new boost::asio::io_service::work(io);

    for (int i = 0; i < NUM_THREAD_IN_POOL; i++) {
        threadpool.create_thread(boost::bind(
                    &boost::asio::io_service::run, &io));
    }

	boost::asio::io_service::work* work_print = new boost::asio::io_service::work(io_print);
	boost::thread thread_print = boost::thread([ = ]() {
		io_print.run();
	});

	int global_seq = 0;

    while (1) {
		int start, end;
		std::cin >> start;
		if (start == -1) {
			break;
		}
		std::cin >> end;
        io.post(boost::bind(&calculate_prime_and_post_print, ++global_seq, start, end));
    }

	delete work;
	threadpool.join_all();

	delete work_print;
	thread_print.join();	
    return 0;
}



