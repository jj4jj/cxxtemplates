
#include "xctmp.h"
#include "iostream"

using namespace std;

int main(){
    xctmp_env_t env;

    env = 20;
    env["hello1"] = 456;
    env["hello2"] = 6.0f;
    env["hello3"] = string("world");
    xctmp_env_t ev2;
    ev2["hello11"] = string("hahaha");
    env["hello4"] = ev2;

    cout << env.str()<< endl;
    return 0;
}
