#include "xctmp.h"
#include <list>
#include <stack>
#include <iostream>
#include <regex>
xctmp_env_t	xctmp_env_t::empty;

static inline int _strtrim(std::string & str, const char * trimchar = " \t\r\n"){
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
static inline  void
_strreplace(std::string & ts, const std::string & p, const std::string & v){
	auto beg = 0;
	auto fnd = ts.find(p, beg);
	while (fnd != std::string::npos){
		ts.replace(fnd, p.length(), v);
		beg = fnd + v.length();
		fnd = ts.find(p, beg);
	}
}

struct xctmp_token_t {
    std::string text; //static from tempalte buffer
    std::string value;//dynamic value binding
    long long	digit; //dyn
	xctmp_func_t  fptr; //dyn
    enum xctmp_token_type {
        TOKEN_ERROR = -1,
        //empty
        TOKEN_NIL = 0,
        //id, num
		TOKEN_ID,
		TOKEN_NUM,
		TOKEN_STRING,
		TOKEN_FUNC,
		//flow contrl 
		TOKEN_CTRL,

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
	bool is_op(){
		return priority() >= 100 && priority() <= 200;
	}
    int priority() const {
		//common	50
		//op 100 - 200	()
		//
        switch (type){
        case TOKEN_EQ:
            return 110;
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
			return 100;
        case TOKEN_BRACKET_RIGHT:
            return 200;
        default:
            return 50;
        }
    }
    bool operator == (const xctmp_token_t & op) const {
        return type == op.type && value == op.value;
    }
};

//need g++4.9+
//static const std::regex   digit_re("^[+-]?[1-9]+");
static const std::regex   var_re("^[a-zA-Z][a-zA-Z0-9_]*(\\.[a-zA-Z][a-zA-Z0-9_]*)*");
static const std::regex   str_re_1("^\"((([^\\\"]\\\")|[^\"])*)\""); //"(([^\"]\")|[^"])*"
static const std::regex   str_re_2("^'((([^\\']\\')|[^'])*)'"); //"(([^\"]\")|[^"])*"
static const std::regex	  ctl_re("^![a-zA-Z]+");

static int 
_expr_parse(std::list<xctmp_token_t> & toklist, const std::string & text){
    xctmp_token_t tok;
    //----------------------------------------
    const char * pcs = text.c_str();
	char * pdend;
	std::smatch m;
    while (*pcs){
        tok.text.clear();
		tok.digit = strtoll(pcs, &pdend, 10);
		if (*pcs == '+' || *pcs == '-' || pdend != pcs){
			if (!toklist.empty() && !toklist.back().is_op()){
				tok.type = (*pcs == '+') ? xctmp_token_t::TOKEN_PLUS : xctmp_token_t::TOKEN_MINUS;
				tok.text.assign(1, *pcs);
				++pcs; //shift one char
			}
			else {
				tok.type = xctmp_token_t::TOKEN_NUM;
				tok.text.assign(pcs, pdend - pcs);
				pcs = pdend;
			}
			toklist.push_back(tok);
			continue;
		}
		tok.type = xctmp_token_t::TOKEN_NIL;
		////////////////////////////////////////
        switch (*pcs){
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
                tok.text = m.str(1);
                pcs += m.length();
				_strreplace(tok.text, "\\\"", "\"");
			}
            else {
                std::cerr << "ilegal string \" style literal :" << pcs << std::endl;
                return -1;
            }
            break;
        case '\'':
            if (std::regex_search(std::string(pcs), m, str_re_2)){
                tok.type = xctmp_token_t::TOKEN_STRING;
                tok.text = m.str(1);
                pcs += m.length();
				_strreplace(tok.text, "\\'", "'");
			}
            else {
                std::cerr << "ilegal string ' style literal :" << pcs << std::endl;
                return -1;
            }
            break;
		case '!':
			if (std::regex_search(std::string(pcs), m, ctl_re)){
				tok.type = xctmp_token_t::TOKEN_STRING;
				tok.text = m.str();
				pcs += m.length();
			}
			else {
				std::cerr << "ilegal ctrl literal :" << pcs << std::endl;
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
		CHUNK_BLOCK_CTL,
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
		else if (text.find("!") == 0){
			type = CHUNK_BLOCK_CTL;
			return _expr_parse(toklist, text);
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
_eval_expr_step(std::stack<const xctmp_token_t*> & opv, std::stack<const xctmp_token_t*> & opt){
    xctmp_token_t result;
    result.type = xctmp_token_t::TOKEN_ERROR;
	if (opv.empty()){
		result.value = "ilegal expression !";
		return result;
	}
    auto op = opt.top();
    opt.pop();
    //binary operator all so far
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
		result.value = "right op value :" + right->text + " value is 0 !";
        return result;

    case xctmp_token_t::TOKEN_FILTER:
        if (left->type != xctmp_token_t::TOKEN_STRING ||
            right->type != xctmp_token_t::TOKEN_FUNC){
            result.value = "filter evaluate param type error !";
            return result;
        }
		result.type = xctmp_token_t::TOKEN_STRING;
        result.value = right->fptr(left->value);
        return result;
    default:
		result.value = "unknown op :" + op->text;
        return result;
    }
}
static inline xctmp_token_t
_eval_expr_reduce(std::stack<const xctmp_token_t*> &opv,
		std::stack<const xctmp_token_t*> & opt,
		const xctmp_token_t & tok,
		std::vector<xctmp_token_t> & _store_token){
	xctmp_token_t error_value;
	error_value.type = xctmp_token_t::TOKEN_ERROR;
	while (!opt.empty()){
		//================================
		auto lop = opt.top();
		if (tok.type == xctmp_token_t::TOKEN_BRACKET_RIGHT){
			if (opt.empty()){
				error_value.value = "bracket not match ! expect ( ";
				return error_value;
			}
			lop = opt.top();
			while (!opt.empty() && lop->type != xctmp_token_t::TOKEN_BRACKET_LEFT){
				auto rtk = _eval_expr_step(opv, opt);
				if (rtk.type == xctmp_token_t::TOKEN_ERROR){
					error_value.value = "evalue step error :" + rtk.value;
					return error_value;
				}
				_store_token.push_back(rtk);
				opv.push(&_store_token.back());
				lop = opt.top();
			}
			if (opt.empty() || lop->type != xctmp_token_t::TOKEN_BRACKET_LEFT){
				error_value.value = "bracket not match ! expect ( ";
				return error_value;
			}
			opt.pop();
			break;
		}
		else if (tok.priority() <= lop->priority()){ //low priority , calculate last
			auto rtk = _eval_expr_step(opv, opt);
			if (rtk.type == xctmp_token_t::TOKEN_ERROR){
				error_value.value = "evalue step error :" + rtk.value;
				return error_value;
			}
			_store_token.push_back(rtk);
			opv.push(&_store_token.back());
		}
		else {
			break;
		}
	}
	if (opt.empty() && opv.size() > 1){
		error_value.value = "expect a operator !";
		return error_value;
	}
	return *opv.top();
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
    xctmp_token_t     result_value;
    result_value.type = xctmp_token_t::TOKEN_ERROR;

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
			auto & val = env.path(tok.text);
			if (val.null()){
				std::cerr << "eval varialbe:" << tok.text << " not found in env !" << std::endl;
			}
			else { 
				switch (val.type){
				case xctmp_env_t::VALUE_INT:
					tok.type = xctmp_token_t::TOKEN_NUM;
					tok.digit = val.uv_.i64;
					break;
				case xctmp_env_t::VALUE_STR:
					tok.type = xctmp_token_t::TOKEN_STRING;
					tok.value = val.str_;
					break;
				case xctmp_env_t::VALUE_FUNC:
					tok.type = xctmp_token_t::TOKEN_FUNC;
					tok.fptr = val.func_;
					break;
				default:
					std::cerr << "warning: eval viralbe:" << tok.text << " not a basic value type !" << std::endl;
					tok.type = xctmp_token_t::TOKEN_STRING;
					tok.value = val.str();
					break;
				}
			}
            opv.push(&tok);
        }
        else { //operator
            if (opt.empty() || tok.type == xctmp_token_t::TOKEN_BRACKET_LEFT){
                opt.push(&tok);
            }
			else if (tok.type == xctmp_token_t::TOKEN_BRACKET_RIGHT ||
					 tok.priority() <= opt.top()->priority()){
                auto rtk = _eval_expr_reduce(opv, opt, tok, _store_token);
                if (rtk.type == xctmp_token_t::TOKEN_ERROR){
                    result_value.value = "evalue step error :" + rtk.value;
                    return result_value;
                }
            }
            else {
                opt.push(&tok); //high level op
            }//end operator branch
        }// end push op
    }//end while
	//====================================================================
	xctmp_token_t _dummy_op;
	_dummy_op.type = xctmp_token_t::TOKEN_EQ;
	result_value = _eval_expr_reduce(opv, opt, _dummy_op, _store_token);
    return result_value;
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
			if (result.type == xctmp_token_t::TOKEN_STRING){
				output.append(result.value);
			}
			else if (result.type == xctmp_token_t::TOKEN_NUM){
				output.append(std::to_string(result.digit));
			}
			else if (result.type == xctmp_token_t::TOKEN_ERROR){
				std::cerr << "render error :" << result.value << " expression error :"<< it->text << std::endl;
				return -1;
			}
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

