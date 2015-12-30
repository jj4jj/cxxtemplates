
#include "xctmp.h"
#include "iostream"
#include <regex>

using namespace std;

string lowercase(const std::string & v){
    return "";
}

int main(){
#if 0
	try {
		regex   re("^([a-zA-Z][a-zA-Z0-9_]*)(\\.[a-zA-Z][a-zA-Z0-9_]*)*", regex::ECMAScript);
		//		regex   re("^[a-zA-Z]", regex::ECMAScript); //must g++4.9+
		string s = "hello.w_orld.yes3 + x + y + z";
		smatch m;
		auto ret = std::regex_search(s, m, re);
		cout << "ret:" << ret << endl;
		if (ret){
			cout << "pos:" << m.position() << " length:" << m.length() << " matched :" << m.str(0) << endl;
		}

		static const std::regex   str_re("\"(([^\\\"]\\\")|[^\"])*\""); //"(([^\"]\")|[^"])*"
		string s2 = "\"hello.\\\"w_orld.yes3\\\" + x + y + z\" a b c";
		m;
		ret = std::regex_search(s2, m, str_re);
		cout << "ret:" << ret << endl;
		if (ret){
			cout << "pos:" << m.position() << " length:" << m.length() << " matched :" << m.str(0) << endl;
		}
	
	}
	catch (regex_error rer){
		cout <<"cl error :" << rer.what() << endl;
		return -1;
	}
    cout << "=================================" << endl;
#endif

    string output;
    xctmp_t * xc =  xctmp_parse("\
    #include \"{{file}}\"\n\
    struct {{struct}}{ \n\
        {{ 5+4.46/7 }}\n\
        {{field.type}} {{field.name | lowercase }}\n\
        {{!if field.array}}\n\
        [{{field.count}}];\n\
        {{!elif field.type = \"message\" }}\n\
        {{!else }}\n\
        {{}}\n\
        {{}}\n\
    };");
    xctmp_env_t env;
    env["file"] = "hello.hpb.h";
    env["struct"] = "HelloST";
    xctmp_env_t field;
    field["type"] = "int32_t";
    field["array"] = 1;
    field["count"] = 24;
    field["name"] = 24;

    env["field"] = field;
    if (!xc){
        return -1;
    }
    xctmp_render(xc, output, env);
    cout << "reder:" << endl
        << output << endl;
    return 0;
}
