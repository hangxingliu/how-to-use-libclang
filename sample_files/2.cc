#include <iostream>
#include <chrono>
#include <string>

#include "string.h"

using namespace std;

typedef chrono::high_resolution_clock Clock;
typedef chrono::time_point<Clock> TimePoint;
typedef chrono::milliseconds Ms;

// HelloWorld
/*
 Comments
 */

class Timer {
	TimePoint t1; string name;
public:
	void tick(const string& name) {
		this->name = name;
		t1 = Clock::now();
		cout << "[.] " << name << " ...\n";
	}
	void tick() {
		auto t = chrono::duration_cast<Ms>(Clock::now() - t1).count();
		cout << "[~] " << name << "'s time usage: " << t << " ms\n";
	}
};


unsigned int limit = 64 * 1024;

void testAppend() {
	string s("");
	for (; s.size() < limit;)
		s += "X";
}
void testFind() {
	string s(limit, 'X');
	for (unsigned i = 0 ; i < limit ; i ++ )
		if (s.find("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0) != string::npos)
			cout << "Find!" << endl;
}

void test1() {
	string s("");
	for (; s.size() < limit;) {
		s += "X";
		if (s.find("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0) != string::npos)
			cout << "Find!" << endl;
	}
}
void test2() {
	unsigned int x = 0;
	string s("");
	for (; x < limit; x++) {
		s += "X";
		if (s.find("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0) != string::npos)
			cout << "Find!" << endl;
	}
}



void testC() {
	char s[limit];
	size_t size = 0;
	while (size < limit) {
		s[size++] = 'X';
		if (memmem(s, size, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)) {
			fprintf(stderr, "Find!\n");
			return;
		}
	}
}

int main_std_string_perform() {
	Timer t;

	t.tick("testAppend"); testAppend(); t.tick();
	t.tick("testFind"); testFind(); t.tick();

	t.tick("test1"); test1(); t.tick();
	t.tick("test2"); test2(); t.tick();
	t.tick("testC"); testC(); t.tick();

	return 0;
}

