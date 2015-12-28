
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
        {{for field in fields }}\n\
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
    if (!xc){
        return -1;
    }
    xctmp_render(xc, output, env);
    cout << "reder:" << endl
        << output << endl;
    return 0;
}
