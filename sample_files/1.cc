#include <stdio.h>
#include <string>

using namespace std;

namespace me {
	int age = 0;
	void sayHi() { printf("Hi!\n"); }
	class Brief {
		string ctx;
	public:
		Brief(string ctx);
		void say();
	};
}

me::Brief::Brief(string ctx): ctx(ctx) {}
void me::Brief::say() { printf("brief: %s\n", ctx.c_str()); }

int main() {
	int& age = me::age;
	age = -1;
	printf("age: %d\n", age);
	me::sayHi();
	me::Brief brief("This is my brief");
	brief.say();
	return 0;
}
