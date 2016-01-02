#include "xctmp.h"
#include "iostream"
#include <regex>
#include <string>
#include <algorithm>

using namespace std;

string lowercase(const std::string & v){
    return "lowercase|" + v;
}

int main(){
#if 0
    cout << "=================================" << endl;
#endif

	//env
	//vars:{"name":value}
	//filters:{"name":address}
	//ext:{"!if":address}
	//!if e
	//text1
	//!else
	//text2
	//!elif
	//!for <id>	in <id>
	//!if <id>._idx = 0
	//!if <id>._idx = 1+tid

    string output;
	xctmp_t * xc = xctmp_parse(
		"{{text}}{{int}}{{float}}{{#comment}}{{var.var}}{{var.var+3}}{{!if var.var == 3}}hello,world!{{!else}}hahhaha, else {{}}{{!for a in var.a}}{{a}}{{}}");
    if (!xc){
        return -1;
    }
	//@filter
	//!ext
	string env = "{\"@lowercase\":\"";
	env += to_string((uint64_t)(void*)lowercase);
	env += "\",\"text\":\"text sample\",\"int\":2346,\"float\":23.57,\"var\":{\"var\":324,\"a\":[2,5,78]}}";
    int ret = xctmp_render(xc, output, env);
    cout << "render:" << ret << endl
        << output << endl;
    return 0;
}
