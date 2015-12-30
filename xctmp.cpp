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
        TOKEN_STRING,
        //+-*/|()
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_PRODUCT,
        TOKEN_DIV,
        TOKEN_FILTER,
        TOKEN_EQ,
        ///////////////////

        TOKEN_BRACKET_LEFT,
        TOKEN_BRACKET_RIGHT,
    } type;
};

//static const std::regex   digit_re("^\\d");
static const std::regex   var_re("^[a-zA-Z][a-zA-Z0-9_]*(\\.[a-zA-Z][a-zA-Z0-9_]*)*");
static const std::regex   str_re_1("\"(([^\\\"]\\\")|[^\"])*\""); //"(([^\"]\")|[^"])*"
static const std::regex   str_re_2("'(([^\\']\\')|[^'])*'"); //"(([^\"]\")|[^"])*"

static int 
_expr_parse(std::list<xctmp_token_t> & toklist, const std::string & text){
    xctmp_token_t tok;
    char * pend;
    //----------------------------------------
    const char * pcs = text.c_str();
    std::smatch m;
    while (true && *pcs){
        tok.text.clear();
        tok.digit = strtof(pcs, &pend);
        if (pend != pcs){ //digit
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
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '-':
            tok.type = xctmp_token_t::TOKEN_MINUS;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '*':
            tok.type = xctmp_token_t::TOKEN_PRODUCT;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '/':
            tok.type = xctmp_token_t::TOKEN_DIV;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '|':
            tok.type = xctmp_token_t::TOKEN_FILTER;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '(':
            tok.type = xctmp_token_t::TOKEN_BRACKET_LEFT;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case ')':
            tok.type = xctmp_token_t::TOKEN_BRACKET_RIGHT;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case '=':
            tok.type = xctmp_token_t::TOKEN_EQ;
            tok.text.assign(1, *pcs);
            ++pcs; //shift one char
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            ++pcs;
            break;
        case '"':
            if (std::regex_search(std::string(pcs), m, str_re_1)){
                tok.type = xctmp_token_t::TOKEN_STRING;
                tok.text = m.str();
                pcs += m.length();
            }
            else {
                std::cerr << "ilegal literal :" << pcs << std::endl;
                return -1;
            }
            break;
        case '\'':
            if (std::regex_search(std::string(pcs), m, str_re_2)){
                tok.type = xctmp_token_t::TOKEN_STRING;
                tok.text = m.str();
                pcs += m.length();
            }
            else {
                std::cerr << "ilegal literal :" << pcs << std::endl;
                return -1;
            }
            break;
        default:
            if (std::regex_search(std::string(pcs), m, var_re)){
                tok.type = xctmp_token_t::TOKEN_ID;
                tok.text = m.str();
                pcs += m.length();
            }
            else {
                std::cerr << "ilegal literal :" << pcs << std::endl;
                return -1;
            }
            break;
        }
        if (tok.type == xctmp_token_t::TOKEN_NIL){
            continue;
        }
        toklist.push_back(tok);
    }
    return 0;
}


struct xctmp_chunk_t {
    std::string text;
    enum xctmp_chunk_type {
        CHUNK_UNKNOWN,
        CHUNK_TEXT,
        CHUNK_EXPR,
        CHUNK_BLOCK_END,
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
        if (text.empty()){
            type = CHUNK_BLOCK_END;
        }
        else if (text.find("#") == 0){
            type = CHUNK_COMMENT;
            text.erase(0);
        }
        else {
            type = CHUNK_EXPR;
            ////////////////////////////////////
            return _expr_parse(toklist, text);
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
    //std::clog << "parsing: "<< chk.text << std::endl;
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

