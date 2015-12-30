#include "xctmp.h"
#include <list>
#include <stack>
#include <iostream>
#include <regex>

static inline int _strtrim(std::string & str, const char * trimchar = " \t\r\n#"){
    auto begmatch = str.begin();
    int count = 0;
    while (begmatch != str.end()){
        if (strchr(trimchar, *begmatch)){
            ++begmatch;
            ++count;
        }
        break;
    }
    if (str.begin() < begmatch){
        str.erase(str.begin(), begmatch);
    }
    //
    auto rbegmatch = str.end();
    while (rbegmatch != str.begin()){
        if (strchr(trimchar, *rbegmatch)){
            --rbegmatch;
            ++count;
        }
        break;
    }
    if (rbegmatch != str.end()){
        str.erase(rbegmatch + 1, str.end());
    }
    return  count;
}
struct xctmp_token_t {
    std::string text;
    long double digit;
    enum xctmp_token_type {
        TOKEN_NIL,
        //id, num
        TOKEN_ID,
        TOKEN_NUM,
        //+-*/|()
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_PRODUCT,
        TOKEN_DIV,
        TOKEN_FILTER,
        TOKEN_BRACKET_LEFT,
        TOKEN_BRACKET_RIGHT,
    } type;
};

struct xctmp_chunk_t {
    std::string text;
    enum xctmp_chunk_type {
        CHUNK_UNKNOWN,
        CHUNK_TEXT,
        CHUNK_VAR,
        CHUNK_EXPR,
        CHUNK_COMMENT,
    } type;
    std::list<xctmp_token_t>    toklist;

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
        _strtrim(text);
        if (text.find("#") == 0){
            type = CHUNK_COMMENT;
            text.erase(0);
        }
        else if (strchr(text.c_str(), '+') ||
            strchr(text.c_str(), '-') ||
            strchr(text.c_str(), '/') ||
            strchr(text.c_str(), '*') ||
            strchr(text.c_str(), '|') ||
            strchr(text.c_str(), ' ') || 
            strchr(text.c_str(), '\t') ||
            strchr(text.c_str(), '\r') ||
            strchr(text.c_str(), '\n')){
            type = CHUNK_EXPR;
            ////////////////////////////////////
            xctmp_token_t tok;
            char * pend;
            //----------------------------------------
            const char * pcs = text.c_str();
            while (true && *pcs){
                tok.digit = strtof(pcs, &pend);
                if (pend != pcs){ //not matched
                    tok.type = xctmp_token_t::TOKEN_NUM;
                    tok.text.assign(pcs, pend - pcs);
                    pcs = pend;
                    toklist.push_back(tok);
                    continue;
                }
                tok.type = xctmp_token_t::TOKEN_NIL;
                ////////////////////////////////////////
                switch (*pcs){
                case '+':
                    tok.type = xctmp_token_t::TOKEN_PLUS;
                    break;
                case '-':
                    tok.type = xctmp_token_t::TOKEN_MINUS;
                    break;
                case '*':
                    tok.type = xctmp_token_t::TOKEN_PRODUCT;
                    break;
                case '/':
                    tok.type = xctmp_token_t::TOKEN_DIV;
                    break;
                case '|':
                    tok.type = xctmp_token_t::TOKEN_FILTER;
                    break;
                case '(':
                    tok.type = xctmp_token_t::TOKEN_BRACKET_LEFT;
                    break;
                case ')':
                    tok.type = xctmp_token_t::TOKEN_BRACKET_RIGHT;
                    break;
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    break;
                default:
                    //match ([a-zA-Z][a-A-Z0-9_]*)[.\1]*
                    const std::regex var_regex("^([a-zA-Z][a-A-Z0-9_]*)(\\.\\1)*");
                    std::smatch m;
                    std::regex_search(text, m, var_regex);
                    if (m.empty()){
                        std::cerr << "ilegal literal :%s!" << text << std::endl;
                        return -1;
                    }
                    else {
                        for (auto & it : m){
                            std::clog << "debug match:" << it << std::endl;
                        }
                        tok.type = xctmp_token_t::TOKEN_ID;
                        tok.text = m[0];
                        pcs += tok.text.length();
                    }
                    break;
                }
                if (tok.type != xctmp_token_t::TOKEN_ID){
                    ++pcs; //shift one char
                }
                if (tok.type == xctmp_token_t::TOKEN_NIL){
                    continue;
                }
                toklist.push_back(tok);
            }
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
_eval_expr(const xctmp_chunk_t & chk, xctmp_env_t & env){
    std::clog << "debug expresion: "<< chk.text << std::endl;
    for (auto & tok : chk.toklist){
        std::clog << tok.text << std::endl;
    }
    return "";
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
            std::clog << "evaluate var :" << it->text << std::endl;
			output.append(env.path(it->text).str());
			break;
		case xctmp_chunk_t::CHUNK_EXPR:
			output.append(_eval_expr(*it, env));
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

