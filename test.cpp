
#include "xctmp.h"
#include "iostream"

using namespace std;

int main(){

	xctmp_env_t	env;
	env[string("hello")] = xctmp_env_value_t("world");
	env.operator[]<int64_t>(string("hello1")) = int64_t(1);
	env.operator[]<void*>(string("hello3")) = (void*)main;
	xctmp_env_t e2;
	e2.operator[]<string>(string("hello")) = string("world");
	e2.operator[]<int64_t>(string("hello1")) = int64_t(1);
	e2.operator[]<void*>(string("hello3")) = (void*)main;

	env.operator[]<xctmp_env_t>(string("hello4")) = e2;

	cout << env.str() << endl;
	cout << env.path("hello") << endl;

}
