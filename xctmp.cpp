#include "xctmp.h"
#include "xctmp_env.hpp"
#include "xctmp_utils.hpp"
#include <string>
#include <unordered_map>
#include <cassert>
#include <functional>
#include <inttypes.h>
#include <list>
#include <stack>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//need g++4.9+
//static const std::regex   digit_re("^[+-]?[1-9]+");
static const std::regex   var_re("^[a-zA-Z][a-zA-Z0-9_]*(\\.[a-zA-Z][a-zA-Z0-9_]*)*");
static const std::regex   str_re_1("^\"((([^\\\"]\\\")|[^\"])*)\""); //"(([^\"]\")|[^"])*"
static const std::regex   str_re_2("^'((([^\\']\\')|[^'])*)'"); //"(([^\"]\")|[^"])*"
static const std::regex	  ctl_re("^!([a-zA-Z]+)");
static const std::regex	  func_re("^@([a-zA-Z]+)");

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
            tok.type = xctmp_token_t::TOKEN_FILT;
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
				tok.type = xctmp_token_t::TOKEN_CTRLER;
				tok.text = m.str(1);
				pcs += m.length();
			}
			else {
				std::cerr << "ilegal ctrl literal :" << pcs << std::endl;
				return -1;
			}
			break;
		case '@':
			if (std::regex_search(std::string(pcs), m, func_re)){
				tok.type = xctmp_token_t::TOKEN_FILTER;
				tok.text = m.str(1);
				pcs += m.length();
			}
			else {
				std::cerr << "ilegal func literal :" << pcs << std::endl;
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
static inline void 
_eval_token(xctmp_token_t & tok, const xctmp_env_t & env){
	if (tok.type != xctmp_token_t::TOKEN_ID){
		return;
	}
	std::string juri = tok.text;
	_strreplace(juri, ".", "/");
	auto v = rapidjson::Pointer(juri.c_str()).Get(env);
	if (v){
		if (v->IsString()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.value = v->GetString();
		}
		else if (v->IsInt64()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetInt64();
		}
		else if (v->IsDouble()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetDouble();
		}
		else if (v->IsBool()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetBool();
		}
		else if (v->IsInt()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetInt();
		}
		else if (v->IsUint()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetUint();
		}
		else if (v->IsUint64()){
			tok.type = xctmp_token_t::TOKEN_NUM;
			tok.digit = v->GetUint64();
		}
	}
}

static inline xctmp_token_t
_eval_expr_step(std::stack<xctmp_token_t*> & opv,
		std::stack<xctmp_token_t*> & opt,
		const xctmp_env_t & env){
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

	_eval_token(*left, env);
	_eval_token(*right, env);

    switch (op->type){
    case xctmp_token_t::TOKEN_EQ:
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = *left == *right ? 1 : 0;
        return result;

    case xctmp_token_t::TOKEN_PLUS:
		if (left->type == xctmp_token_t::TOKEN_NUM &&
			right->type == xctmp_token_t::TOKEN_NUM){
			result.type = xctmp_token_t::TOKEN_NUM;
			result.digit = left->digit + right->digit;
			return result;
		}
		if (left->type == xctmp_token_t::TOKEN_NUM){
			result.value += std::to_string(left->digit);
        }
		else {
			result.value += left->value;
		}
		if (right->type == xctmp_token_t::TOKEN_NUM){
			result.value += std::to_string(right->digit);
		}
		else {
			result.value += right->value;
		}
		result.type = xctmp_token_t::TOKEN_STRING;
        return result;

    case xctmp_token_t::TOKEN_MINUS:
        //must be num
		if (left->type != xctmp_token_t::TOKEN_NUM ||
			right->type != xctmp_token_t::TOKEN_NUM){
			result.value = "minus opetaor param type error !";
			return result;
		}
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = left->digit - right->digit;
        return result;

    case xctmp_token_t::TOKEN_PRODUCT:
		if (left->type == xctmp_token_t::TOKEN_NUM &&
			right->type == xctmp_token_t::TOKEN_NUM){
			result.type = xctmp_token_t::TOKEN_NUM;
			result.digit = left->digit * right->digit;
			return result;
		}
		if (left->type == xctmp_token_t::TOKEN_STRING &&
			right->type == xctmp_token_t::TOKEN_NUM){
			int n = right->digit;
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = "";
			while (n-- > 0){
				result.value += left->value;
			}
			return result;
		}
		if (right->type == xctmp_token_t::TOKEN_STRING &&
			left->type == xctmp_token_t::TOKEN_NUM){
			int n = left->digit;
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = "";
			while (n-- > 0){
				result.value += right->value;
			}
			return result;
		}
        result.value = "product operator param error !";
        return result;

    case xctmp_token_t::TOKEN_DIV:
		if (left->type != xctmp_token_t::TOKEN_NUM ||
			right->type != xctmp_token_t::TOKEN_NUM){
			result.value = "div opetaor param type error !";
			return result;
		}
		result.type = xctmp_token_t::TOKEN_NUM;
        if (right->digit > 0){
            result.digit = left->digit / right->digit;
            return result;
        }
		result.value = "right op value :" + right->text + " value is 0 !";
        return result;

    case xctmp_token_t::TOKEN_FILT:
        if (left->type != xctmp_token_t::TOKEN_STRING ||
            right->type != xctmp_token_t::TOKEN_FILTER){
            result.value = "filter evaluate param type error !";
            return result;
        }
		else {
			xctmp_filter_t fpfilter;
			auto v = rapidjson::Pointer(right->value.c_str()).Get(env);
			if (!v){
				result.value = "filter right param not found in env!";
				return result;
			}
			sscanf(v->GetString(), "%p", &fpfilter);
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = fpfilter(left->value);
			return result;
		}
    default:
		result.value = "unknown operator :" + op->text;
        return result;
    }
}
static inline xctmp_token_t
_eval_expr_reduce(std::stack<xctmp_token_t*> &opv,
		std::stack<xctmp_token_t*> & opt,
		const xctmp_token_t & tok,
		std::vector<xctmp_token_t> & _store_token,
		const xctmp_env_t & env){
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
				auto rtk = _eval_expr_step(opv, opt, env);
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
			auto rtk = _eval_expr_step(opv, opt, env);
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

    std::stack<xctmp_token_t*>     opv;
    std::stack<xctmp_token_t*>     opt;
    std::vector<xctmp_token_t>           _store_token;
    auto tokenlist = chk.toklist;
    auto it = tokenlist.begin();
    while(it != tokenlist.end()){	
		auto & tok = *(it++);
        if (tok.type == xctmp_token_t::TOKEN_NUM ||
            tok.type == xctmp_token_t::TOKEN_STRING ||
			tok.type == xctmp_token_t::TOKEN_ID ){
            opv.push(&tok);
        }
        else { //operator
            if (opt.empty() || tok.type == xctmp_token_t::TOKEN_BRACKET_LEFT){
                opt.push(&tok);
            }
			else if (tok.type == xctmp_token_t::TOKEN_BRACKET_RIGHT ||
					 tok.priority() <= opt.top()->priority()){
                auto rtk = _eval_expr_reduce(opv, opt, tok, _store_token, env);
                if (rtk.type == xctmp_token_t::TOKEN_ERROR){
                    result_value.value = "evaluation step error :" + rtk.value;
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
	result_value = _eval_expr_reduce(opv, opt, _dummy_op, _store_token, env);
    return result_value;
}
int       
xctmp_render(xctmp_t * xc, std::string & output, const std::string & jsenv){
	auto it = xc->chunk_list.begin();
    xctmp_token_t result;
	xctmp_env_t env;
	int ret = env.loads(jsenv.c_str());
	if (ret){
		std::cerr << "json load error !" << std::endl;
		return ret;
	}
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

//=====================================================================

bool xctmp_token_t::is_op(){
	return priority() >= 100 && priority() <= 200;
}
int xctmp_token_t::priority() const {
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
	case TOKEN_FILT:
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
bool xctmp_token_t::operator == (const xctmp_token_t & op) const {
	return type == op.type && value == op.value;
}

//========================================
xctmp_chunk_t::xctmp_chunk_t(){
	type = CHUNK_UNKNOWN;
}
xctmp_chunk_t::xctmp_chunk_t(const std::string & t){
	text = t;
	type = CHUNK_TEXT;
}
int     xctmp_chunk_t::parse(){
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
		return _expr_parse(toklist, text);
	}
	return 0;
}



