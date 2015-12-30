
#include "xctmp.h"
#include "iostream"
using namespace std;

string lowercase(const std::string & v){
    return "";
}

int main(){
    string output;
    xctmp_t * xc =  xctmp_parse("\
    #include \"{{file}}\"\n\
    struct {{struct}}{ \n\
        {{ 5+4.46/7 }}\n\
        {{field.type}} {{field.name | lowercase }}\n\
        {{if field.array}}\n\
        [{{field.count}}];\n\
        {{elif field.type == \"message\" }}\n\
        {{ else }}\n\
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
