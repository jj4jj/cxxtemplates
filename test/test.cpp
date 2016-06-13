#include "xctmp.h"
#include <iostream>
#include <regex>
#include <string>
#include <algorithm>
#include <fstream>
using namespace std;

string lowercase(const std::string & v){
    return "lowercase|" + v;
}

int main(){
#if 1
	smatch	res;
	regex	tre("^!([a-zA-Z]+)");
	string sss = "!else if";
	//const char * pcs = "!else if"; testing regex bug
	if (regex_search(sss, res, tre)){
		cout << "search :" << res.str(1) << std::endl;
	}
    cout << "=================================" << endl;
#endif
    string output;

    ifstream ifs("test.xtp", ios::in);
    char buffer[2048] = { 0 };
    ifs.getline(buffer, sizeof(buffer), 0);
    cout << "read some:" << buffer << endl;
	xctmp_t * xc = xctmp_parse(buffer);
    if (!xc){
        return -1;
    }
   
	//@filter
	//!ext
    xctmp_push_filter(xc, "lowercase", lowercase);
	string env = "{\"text\":\"text sample\",\"int\":2346,\"float\":23.57,\"var\":{\"var\":324,\"a\":[2,5,78]}}";
    cout << "env:" << endl << env << endl;
    int ret = xctmp_render(xc, output, env);
    cout << "render state :" << ret <<  endl << "===============================" << endl
        << output << endl;
    return 0;
}
