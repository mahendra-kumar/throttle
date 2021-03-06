﻿// throttle.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <limits>
#include <queue>
#include <numeric>
#include <chrono>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "date.h"

std::chrono::seconds oneSec(1);

using DATA = struct _DATA
{
	uint64_t num;
	uint8_t priority;
};

struct LessThanByPriority
{
	bool operator()(const DATA& lhs, const DATA& rhs) const
	{
		if (lhs.priority == rhs.priority)
			return  lhs.num > rhs.num; // num check not necessary
		return lhs.priority < rhs.priority;
	}
};

using DATAQ = std::priority_queue<DATA, std::vector<DATA>, LessThanByPriority>;

uint8_t priorityOf(uint64_t n) {
	uint8_t ret = 0;
	if (n % 7 == 0) ret = 1;
	return ret;
}

std::mutex dataMutex;

class Producer {
public:
	Producer(DATAQ &dq, std::chrono::seconds howLong = std::chrono::seconds(5)) : _dq(dq), _howLong(howLong) {}
	void operator()() {
		produce();
	}
	void SetSpeed(unsigned n) { speed = n; }
protected:
	void produce() {
		auto start = std::chrono::system_clock::now();
		while (num < std::numeric_limits<uint64_t>::max()) {
			DATA data;
			data.num = num++;
			data.priority = priorityOf(data.num);
			// Producer "speed" 
			std::this_thread::sleep_for(std::chrono::milliseconds(100/ speed));
			{
				std::lock_guard<std::mutex> data_lock(dataMutex);
				_dq.push(data);
			}
			if (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start) >= std::chrono::duration_cast<std::chrono::nanoseconds>(_howLong)) {
				data.num = static_cast<uint64_t>(-1);
				data.priority = 0;
				{
					std::lock_guard<std::mutex> data_lock(dataMutex);
					_dq.push(data);
				}
				break;
			}
		}
	}
private:
	uint64_t num = 1;
	DATAQ &_dq;
	std::chrono::seconds _howLong;
	unsigned speed = 50;
};

class TPSConsumer {
public:
	TPSConsumer(DATAQ &dq) : _dq(dq) {}
	void operator()() {
		 consume();
	}
	void SetCutOff(uint16_t co) { cutOff = co; }
protected:
	void consume() {
		while (true) {
			// ensure producer is faster than consumer or if not, handle accordingly
			while (!_dq.size()) {
				std::this_thread::sleep_for(std::chrono::microseconds((rand() % 2) + 1));
			}
			DATA data;
			{
				std::lock_guard<std::mutex> data_lock(dataMutex);
				data = _dq.top();
			}
			// -1 flag indicates end of data
			if (data.num == static_cast<uint64_t>(-1)) {
				std::cout << ss.str();
				ss.str(std::string());
				break;
			}
			// write a line for each message with time
			auto now = std::chrono::system_clock::now();
			if (timeLog.size() < cutOff) {
				process(data, now);
				{
					std::lock_guard<std::mutex> data_lock(dataMutex);
					_dq.pop();
				}
				timeLog.push(now);
			} else {
				auto lastTime = timeLog.front();
				auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime);
				if (diff > std::chrono::duration_cast<std::chrono::nanoseconds>(oneSec)) {
					process(data, now);
					{
						std::lock_guard<std::mutex> data_lock(dataMutex);
						_dq.pop();
					}
					timeLog.pop();
					timeLog.push(now);
				}
			}
		}
	}
private:
	void process(const DATA &data, std::chrono::system_clock::time_point &now) {
		using namespace date;
		if (sw) {
			ss3 << format("%T", floor<std::chrono::microseconds>(now)) << ":\t" << data.num << " (" << static_cast<int>(data.priority) << ")" << "\n";
		} else {
			if (!ss3.str().empty()) {
				ss = std::move(ss3);
				ss3.str(std::string());;
			}
			ss << format("%T", floor<std::chrono::microseconds>(now)) << ":\t" << data.num << " (" << static_cast<int>(data.priority) << ")" << "\n";
		}

		if (!iobusy && ss.str().length() > 4 * 1024 /* 1024*/) {
			sw = true;
			iobusy = true;
			std::thread t([this]() {
				ss2 = std::move(ss);
				ss.str(std::string());;
				sw = false;
				std::cout << ss2.str();
				iobusy = false;
			});
			t.detach();
		}
	}
	DATAQ &_dq;
	uint16_t cutOff = 100;	// messages per second
	std::queue<std::chrono::system_clock::time_point> timeLog;
	std::stringstream ss;
	std::stringstream ss2;
	std::stringstream ss3;
	std::atomic_bool sw{ false };
	std::atomic_bool iobusy{ false };
};

int main(int argc, char **argv)
{
	DATAQ pq;
	std::chrono::seconds timeForProducer= std::chrono::seconds(5);
	TPSConsumer consumer(pq);
	if (argc > 2) {
		int val = atoi(argv[2]);
		if (val > 1) timeForProducer = std::chrono::seconds(val);
	}
	if (argc > 1) {
		int val = atoi(argv[1]);
		if (val > 0) consumer.SetCutOff(val);
	}
	Producer producer(pq, timeForProducer);
	if (argc > 3) {
		int val = atoi(argv[3]);
		if (val > 1) producer.SetSpeed(val);
	}
	std::thread t2(std::ref(consumer));
	std::thread t(std::ref(producer));
	t.join();
	t2.join();
	return 0;
}

