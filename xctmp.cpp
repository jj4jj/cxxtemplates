#include "xctmp.h"
#include <list>
#include <stack>
#include <iostream>

struct xctmp_chunk_t {
    std::string text;
    enum xctmp_chunk_type {
        CHUNK_UNKNOWN,
        CHUNK_TEXT,
        CHUNK_VAR,
        CHUNK_EXPR,
        CHUNK_COMMENT,
    } type;
    xctmp_chunk_t(){
        type = CHUNK_UNKNOWN;
    }
    xctmp_chunk_t(const std::string & t){
        text = t;
        type = CHUNK_TEXT;
    }
    int     parse(){
        if (type == CHUNK_TEXT){
            return 0;
        }
        if (text.find("#") == 0){
            type = CHUNK_COMMENT;
        }
        else if (strchr(text.c_str(), '+') ||
            strchr(text.c_str(), '-') ||
            strchr(text.c_str(), '/') ||
            strchr(text.c_str(), '*') ||
            strchr(text.c_str(), '|')){
            type = CHUNK_EXPR;
        }
        else {
            type = CHUNK_VAR;
        }
        return 0;
    }
};
struct xctmp_t {
	std::string					text;
	std::list<xctmp_chunk_t>	chunk_list;
};

xctmp_t * 
xctmp_parse(const std::string & text){
    //parse generate chunk list
    std::string::size_type beg = 0, fnd = 0;
    std::list<xctmp_chunk_t>    chklist;
    int ret = 0;
    while (true){
        fnd = text.find("{{", beg);
        if (fnd == std::string::npos){
            xctmp_chunk_t xcc(text.substr(beg));
            chklist.push_back(xcc);
            break;
        }
        else { //fnd it
            xctmp_chunk_t xcc(text.substr(beg, fnd - beg));
            chklist.push_back(xcc);
            beg = fnd + 2;
        }
        //must be transing closing
        fnd = text.find("}}", beg);
        if (fnd == std::string::npos){
            std::cerr << "expect }} from pos:" << beg << " text:"<< text.substr(beg, 32) << " ..." << std::endl;
            ret = -1;
            break;
        }
        else {
            //close one
            xctmp_chunk_t xcc;
            xcc.text = text.substr(beg, fnd - beg); //parse later
            chklist.push_back(xcc);
            beg = fnd + 2;
        }
    }
    if (ret){
        return nullptr;
    }

    for (auto & xcc: chklist){
        if (xcc.parse()){
            std::cerr << "parse chunk error text:"<< xcc.text << std::endl;
            ret -= 1;
        }
    }

    if (ret){
        return nullptr;
    }

    xctmp_t * xc = new xctmp_t();
    xc->text = text;
    xc->chunk_list = chklist;
	return xc;
}
static inline std::string 
_eval_expr(const std::string & expr, xctmp_env_t & env){
	return "todo";
}
int       
xctmp_render(xctmp_t * xc, std::string & output, xctmp_env_t & env){
	auto it = xc->chunk_list.begin();
    for (; it != xc->chunk_list.end(); ++it){
		switch (it->type){
		case xctmp_chunk_t::CHUNK_TEXT:
			output.append(it->text);
			break;
		case xctmp_chunk_t::CHUNK_VAR:
			output.append(env.path(it->text).str());
			break;
		case xctmp_chunk_t::CHUNK_EXPR:
			output.append(_eval_expr(it->text, env));
			break;
		case xctmp_chunk_t::CHUNK_COMMENT:
			break;
		}
	}
	return 0;
}
void      
xctmp_destroy(xctmp_t * xc){
	delete xc;
}

