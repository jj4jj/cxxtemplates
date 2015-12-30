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
    std::string value;
    long double digit;
    enum xctmp_token_type {
        TOKEN_ERROR = -1,
        //empty
        TOKEN_NIL = 0,
        //id, num
        TOKEN_ID,
        TOKEN_NUM,
        TOKEN_STRING,
        TOKEN_FUNC,

        //+-*/|() -----operator--------
        TOKEN_EQ ,
        TOKEN_PLUS ,
        TOKEN_MINUS ,
        TOKEN_PRODUCT,
        TOKEN_DIV ,
        TOKEN_FILTER ,
        ///////////////////
        TOKEN_BRACKET_LEFT ,
        TOKEN_BRACKET_RIGHT,
    } type;
    int priority() const {
        switch (type){
        case TOKEN_EQ:
            return 100;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 150;
        case TOKEN_PRODUCT:
        case TOKEN_DIV:
            return 160;
        case TOKEN_FILTER:
            return 180;
                ///////////////////
        case TOKEN_BRACKET_LEFT:
        case TOKEN_BRACKET_RIGHT:
            return 200;
        default:
            return 0;
        }
    }
    bool operator == (const xctmp_token_t & op) const {
        return type == op.type && value == op.value;
    }
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

static inline xctmp_token_t
_eval_expr_reduce(std::stack<const xctmp_token_t*> &opv,
    std::stack<const xctmp_token_t*> & opt, const xctmp_token_t & tok);

static inline xctmp_token_t
_eval_expr_step(std::stack<const xctmp_token_t*> & opv, std::stack<const xctmp_token_t*> & opt){
    xctmp_token_t result;
    result.type = xctmp_token_t::TOKEN_ERROR;

    if (opv.size() < 2 || opt.size() < 1){
        result.text = "ilegal expression !";
        return result;
    }
    auto op = opt.top();
    opt.pop();

    if (op->type == xctmp_token_t::TOKEN_BRACKET_RIGHT){
        result = _eval_expr_reduce(opv, opt, *op);
        if (result.type == xctmp_token_t::TOKEN_ERROR){
            return result;
        }
        else if (opt.empty() || opt.top()->type != xctmp_token_t::TOKEN_BRACKET_RIGHT){
            result.type = xctmp_token_t::TOKEN_ERROR;
            result.text = "bracket pair not matched !";
            return result;
        }
        assert(opt.top()->type == xctmp_token_t::TOKEN_BRACKET_RIGHT);
        opt.pop();
        return result;
    }
    //binary operator
    auto right = opv.top();
    opv.pop();
    auto left = opv.top();
    opv.pop();
    switch (op->type){
    case xctmp_token_t::TOKEN_EQ:
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = *left == *right ? 1.0 : 0;
        return result;

    case xctmp_token_t::TOKEN_PLUS:
        if (left->type == xctmp_token_t::TOKEN_STRING){
            result.type = xctmp_token_t::TOKEN_STRING;
            result.value = left->value + right->value;
            return result;
        }
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = left->digit + right->digit;
        return result;

    case xctmp_token_t::TOKEN_MINUS:
        //must be num
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = left->digit - right->digit;
        return result;

    case xctmp_token_t::TOKEN_PRODUCT:
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = left->digit * right->digit;
        return result;

    case xctmp_token_t::TOKEN_DIV:
        result.type = xctmp_token_t::TOKEN_NUM;
        if (right->digit > 0){
            result.digit = left->digit / right->digit;
            return result;
        }
        result.text = "right op value :" + right->text + " value is 0 !";
        return result;

    case xctmp_token_t::TOKEN_FILTER:
        if (left->type != xctmp_token_t::TOKEN_STRING ||
            right->type != xctmp_token_t::TOKEN_FUNC){
            result.text = "filter evaluate param type error !";
            return result;
        }
        xctmp_func_t pf;
        sscanf(right->value.c_str(), "%p", &pf);
        result.value = pf(left->value);
        return result;
    default:
        result.text = "unknown op :" + op->text;
        return result;
    }
}

static inline xctmp_token_t
_eval_expr_reduce(std::stack<const xctmp_token_t*> &opv,
                    std::stack<const xctmp_token_t*> & opt, const xctmp_token_t & tok){
    using std::vector;
    vector<xctmp_token_t>   _store;
    xctmp_token_t result;
    result.type = xctmp_token_t::TOKEN_ERROR;
    while (!opt.empty()){
        auto lop = opt.top();
        if (tok.priority() <= lop->priority()){ //low priority , calculate last
            auto rtk = _eval_expr_step(opv, opt);
            if (rtk.type == xctmp_token_t::TOKEN_ERROR){
                result.text = "evalue step error :" + rtk.value;
                return result;
            }
            _store.push_back(rtk);
            opv.push(&_store.back());
        }
    }
    result = *opv.top();
    opv.pop();
    return result;
}

static inline xctmp_token_t
_eval_expr(const xctmp_chunk_t & chk, const xctmp_env_t & env){
    //std::clog << "parsing: "<< chk.text << std::endl;
    //e:    digit
    //      var
    //      e+e
    //      e-e
    //      e/e
    //      e*e
    //      (e)
    //      e | filter
    //filter:   var
    //var:      id
    //cop
    //if cop >= lop
    //push cop
    //else
    //eval  lop
    //push  cop
    xctmp_token_t     value;
    value.type = xctmp_token_t::TOKEN_ERROR;

    std::stack<const xctmp_token_t*>     opv;
    std::stack<const xctmp_token_t*>     opt;
    std::vector<xctmp_token_t>           _store_token;
    auto tokenlist = chk.toklist;
    auto it = tokenlist.begin();
    while(it != tokenlist.end()){
        auto & tok = *(it++);
        if (tok.type == xctmp_token_t::TOKEN_NUM ||
            tok.type == xctmp_token_t::TOKEN_STRING){
            tok.value = tok.text;
            opv.push(&tok);
        }
        else if (tok.type == xctmp_token_t::TOKEN_ID){
            tok.value = env.path(tok.text).str();
            opv.push(&tok);
        }
        else {
            if (opt.empty()){
                opt.push(&tok);
            }
            else {
                auto lop = opt.top();
                if (tok.priority() <= lop->priority()){ //low priority , calculate last
                    auto rtk = _eval_expr_reduce(opv, opt, tok);
                    if (rtk.type == xctmp_token_t::TOKEN_ERROR){
                        value.text = "evalue step error :" + rtk.value;
                        return value;
                    }
                    _store_token.push_back(rtk);
                    opv.push(&_store_token.back());
                }
                else {
                    opt.push(&tok);
                }//end if push token that is operator
            }//end if push token
        }// end while
    }
    return value;
}
int       
xctmp_render(xctmp_t * xc, std::string & output, xctmp_env_t & env){
	auto it = xc->chunk_list.begin();
    xctmp_token_t result;
    for (; it != xc->chunk_list.end(); ++it){
		switch (it->type){
		case xctmp_chunk_t::CHUNK_TEXT:
			output.append(it->text);
			break;
		case xctmp_chunk_t::CHUNK_EXPR:
            result = _eval_expr(*it, env);
			output.append();
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

