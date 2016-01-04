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
#include <memory>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//need g++4.9+
//static const std::regex   digit_re("^[+-]?[1-9]+");
static const std::regex   var_re("^[a-zA-Z][a-zA-Z0-9_]*(\\.[a-zA-Z0-9_]+)*");
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
            if ((*pcs == '+' || *pcs == '-') && !toklist.empty() && !toklist.back().is_op()){
				tok.type = (*pcs == '+') ? xctmp_token_t::TOKEN_PLUS : xctmp_token_t::TOKEN_MINUS;
				tok.text.assign(1, *pcs);
				++pcs; //shift one char
			}
			else if (pdend != pcs){
				tok.type = xctmp_token_t::TOKEN_NUM;
				tok.text.assign(pcs, pdend - pcs);
				pcs = pdend;
			}
            else {
                std::cerr << "ambigous literal(operator +/- or number(decimalism) ?): " << pcs << std::endl;
                return -1;
            }
			toklist.push_back(tok);
			continue;
		}
		tok.type = xctmp_token_t::TOKEN_NIL;
		std::string strtoks = pcs;
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
		case '<':
			tok.type = xctmp_token_t::TOKEN_LT;
			tok.text.assign(1, *pcs);
			++pcs; //shift one char
			break;
		case '>':
			tok.type = xctmp_token_t::TOKEN_GT;
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
			if (std::regex_search(strtoks, m, str_re_1)){
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
			if (std::regex_search(strtoks, m, str_re_2)){
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
			if (std::regex_search(strtoks, m, ctl_re)){
				tok.type = xctmp_token_t::TOKEN_CTRLER;
				tok.text = m.str(1);
				//std::clog << "parsed a control tag: " << tok.text << " match: "<< m.str(1) << " pcs: "<< pcs << std::endl;
				pcs += m.length();
			}
			else {
				std::cerr << "ilegal ctrl literal :" << pcs << std::endl;
				return -1;
			}
			break;
		case '@':
			if (std::regex_search(strtoks, m, func_re)){
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
			if (std::regex_search(strtoks, m, var_re)){
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
	typedef std::list<xctmp_chunk_t>::iterator chunk_list_itr_t;
};
struct xctmp_chunk_env_t {
	xctmp_t::chunk_list_itr_t	itr; //point to chunk
	struct loop_env_t {
		int						idx;
		int						total;
		std::string				itrname;
		std::string				arrayname;
		const rapidjson::Value	*value;
	} loop_;
	struct branch_env_t {
		bool					pred;
	} branch_;
	xctmp_chunk_env_t(){
		loop_.idx = 0;
		loop_.total = 0;
		loop_.itrname = "";
		loop_.arrayname = "";
		loop_.value = nullptr;
		branch_.pred = false;		
	}
};
struct xctmp_env_t {
	json_doc_t							global;
	std::list<xctmp_chunk_env_t>		chunks;
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

static inline const rapidjson::Value *
_find_value_by_path(const std::string & pth, const rapidjson::Value & v){
	return rapidjson::Pointer(pth.c_str()).Get(v);
}

static inline const rapidjson::Value *
_find_env_value(const std::string & jpuri, int idx, const xctmp_env_t & env){
	auto it = env.chunks.rbegin();
	const rapidjson::Value *v = nullptr;
	while (it != env.chunks.rend()){
		if (!it->loop_.itrname.empty() && jpuri.find(it->loop_.itrname) == 0){ //...x..
			if (it->loop_.itrname == jpuri){
				return it->loop_.value;
			}
			auto jpt = jpuri.substr(it->loop_.itrname.length());
			_strreplace(jpt, ".", "/");
			v = _find_value_by_path(jpt, *it->loop_.value);
			if (v){
				return v;
			}
		}
		it++;
	}
	std::string jpt = "/" + jpuri;
	if (idx >= 0){
		jpt += "/" + std::to_string(idx);
	}
	_strreplace(jpt, ".", "/");
	v = _find_value_by_path(jpt, env.global);
	return v;
}

static inline xctmp_token_t
_eval_token(const xctmp_token_t & rtok_, const xctmp_env_t & env){
	if (rtok_.type != xctmp_token_t::TOKEN_ID){
		return rtok_;
	}
	xctmp_token_t tok = rtok_;
	tok.type = xctmp_token_t::TOKEN_ERROR;
	auto v = _find_env_value(rtok_.text, -1, env);
	if (v){
		if (v->IsString()){
			tok.type = xctmp_token_t::TOKEN_STRING;
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
		else {
			tok.value = "error type the id:" + tok.text;
		}
		return tok;
	}
	else {
		std::string juri = "/@" + tok.text;
		_strreplace(juri, ".", "/");
		auto v = rapidjson::Pointer(juri.c_str()).Get(env.global);
		if (v){
			if (v->IsString()){
				tok.type = xctmp_token_t::TOKEN_FILTER;
				tok.value = v->GetString();
				assert(sizeof(long long) == sizeof(void *));
				sscanf(tok.value.c_str(), "%llu", &tok.digit);
			}
			else if (v->IsUint64()){
				tok.type = xctmp_token_t::TOKEN_FILTER;
				tok.digit = v->GetUint64();
			}
			else {
				tok.value = "error filter type of id:" + tok.text ;
			}
			return tok;
		}
	}
	tok.value = "not found the env entry of id:" + tok.text;
	return tok;
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

	auto lv = _eval_token(*left, env);
	auto rv = _eval_token(*right, env);
	if (lv.type == xctmp_token_t::TOKEN_ERROR ||
		rv.type == xctmp_token_t::TOKEN_ERROR){
		result.value = "error token value get => left: " + lv.text + " value: " + lv.value +
						" right: " + rv.text + " value: " + rv.value;
		return result;
	}

    switch (op->type){
    case xctmp_token_t::TOKEN_EQ:
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = (lv == rv) ? 1 : 0;
        return result;
	case xctmp_token_t::TOKEN_LT:
		result.type = xctmp_token_t::TOKEN_NUM;
		result.digit = (lv < rv) ? 1 : 0;
		return result;
	case xctmp_token_t::TOKEN_GT:
		result.type = xctmp_token_t::TOKEN_NUM;
		result.digit = (lv > rv) ? 1 : 0;
		return result;

    case xctmp_token_t::TOKEN_PLUS:
		if (lv.type == xctmp_token_t::TOKEN_NUM &&
			rv.type == xctmp_token_t::TOKEN_NUM){
			result.type = xctmp_token_t::TOKEN_NUM;
			result.digit = lv.digit + rv.digit;
			return result;
		}
		if (lv.type == xctmp_token_t::TOKEN_NUM){
			result.value += std::to_string(lv.digit);
        }
		else {
			result.value += lv.value;
		}
		if (rv.type == xctmp_token_t::TOKEN_NUM){
			result.value += std::to_string(rv.digit);
		}
		else {
			result.value += rv.value;
		}
		result.type = xctmp_token_t::TOKEN_STRING;
        return result;

    case xctmp_token_t::TOKEN_MINUS:
        //must be num
		if (lv.type != xctmp_token_t::TOKEN_NUM ||
			rv.type != xctmp_token_t::TOKEN_NUM){
			result.value = "minus opetaor param type error !";
			return result;
		}
        result.type = xctmp_token_t::TOKEN_NUM;
        result.digit = lv.digit - rv.digit;
        return result;

    case xctmp_token_t::TOKEN_PRODUCT:
		if (lv.type == xctmp_token_t::TOKEN_NUM &&
			rv.type == xctmp_token_t::TOKEN_NUM){
			result.type = xctmp_token_t::TOKEN_NUM;
			result.digit = lv.digit * rv.digit;
			return result;
		}
		if (lv.type == xctmp_token_t::TOKEN_STRING &&
			rv.type == xctmp_token_t::TOKEN_NUM){
			int n = rv.digit;
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = "";
			while (n-- > 0){
				result.value += lv.value;
			}
			return result;
		}
		if (rv.type == xctmp_token_t::TOKEN_STRING &&
			lv.type == xctmp_token_t::TOKEN_NUM){
			int n = lv.digit;
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = "";
			while (n-- > 0){
				result.value += rv.value;
			}
			return result;
		}
        result.value = "product operator param error !";
        return result;

    case xctmp_token_t::TOKEN_DIV:
		if (lv.type != xctmp_token_t::TOKEN_NUM ||
			rv.type != xctmp_token_t::TOKEN_NUM){
			result.value = "div opetaor param type error !";
			return result;
		}
		result.type = xctmp_token_t::TOKEN_NUM;
        if (rv.digit > 0){
            result.digit = lv.digit / rv.digit;
            return result;
        }
		result.value = "right op value :" + rv.text + " value is 0 !";
        return result;

    case xctmp_token_t::TOKEN_FILT:
        if (rv.type != xctmp_token_t::TOKEN_FILTER){
            result.value = "filter evaluate param type error !";
            return result;
        }
		else {
			xctmp_filter_t fpfilter = (xctmp_filter_t)rv.digit;
			std::string leftstr = lv.value;
			if (lv.type == xctmp_token_t::TOKEN_NUM){
				leftstr = std::to_string(lv.digit);
			}
			result.type = xctmp_token_t::TOKEN_STRING;
			result.value = fpfilter(leftstr);
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
	if (opv.size() == 1){ //just one opvalue
		_eval_token(*opv.top(), env);
	}
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
	return _eval_token(*opv.top(), env);
}

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
static inline xctmp_token_t
_eval_expr(const xctmp_chunk_t & chk, const xctmp_env_t & env){
    //std::clog << "parsing: "<< chk.text << std::endl;
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
			if (!tok.is_keywords()){
				opv.push(&tok);
			}
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

static inline int 
_render_begin_block(xctmp_t* xc, std::string & output, xctmp_t::chunk_list_itr_t & chunkit, xctmp_env_t & env){
	auto tokit = chunkit->toklist.begin();
	auto & tag = *tokit;
	++tokit;
	if (tag.text == "if" || tag.text == "elif"){	//if expr
		auto tok = _eval_expr(*chunkit, env);//evaluation
		if (tok.type != xctmp_token_t::TOKEN_NUM){
			std::cerr << "eval expression error chunk:" << chunkit->text << std::endl;
			return -1;
		}
		if (tag.text == "if"){ //create if env
			env.chunks.push_back(xctmp_chunk_env_t());
			auto & chenv = env.chunks.back();
			chenv.itr = chunkit;
		}
		if (tok.digit != 0){ //true , sequence excute
			env.chunks.back().branch_.pred = true;
			return 0;
		}
		else {
			env.chunks.back().branch_.pred = false; // until to next branch
			++chunkit;
			while (chunkit != xc->chunk_list.end()){
				if (chunkit->type == xctmp_chunk_t::CHUNK_BLOCK_CTL)
				{
					if (chunkit->toklist.begin()->text == "else" ||
						chunkit->toklist.begin()->text == "elif"){
						--chunkit;
						return 0;
					}
				}
				else if (chunkit->type == xctmp_chunk_t::CHUNK_BLOCK_END){
					--chunkit;
					return 0;
				}
				++chunkit;
			}
			std::cerr << "error branch syntax , not found a pair match if else/elif/end" << std::endl;
			return -1;
		}		
	}
	else if (tag.text == "else"){ //else
		if (env.chunks.back().branch_.pred){ //ignore else (false)
			++chunkit;
			while (chunkit != xc->chunk_list.end()){
				if (chunkit->type == xctmp_chunk_t::CHUNK_BLOCK_END){
					--chunkit;
					return 0;
				}
				++chunkit;
			}
			std::cerr << "not match the branch syntax , not found block end '{{}}'";
			return -1;
		}
		return 0;
	}
	else if (tag.text == "for"){ //for <v> in <v>(array)
		//create env
		auto itrname = tokit->text;
		if (tokit->type != xctmp_token_t::TOKEN_ID){
			std::cerr << "error for syntax the itr name not a id !" << tokit->text << std::endl;
			return -1;
		}
		tokit++;
		//in
		if (tokit->text != "in"){
			std::cerr << "error for syntax expect 'in' literal !" << tokit->text << std::endl;
			return -1;
		}
		tokit++;
		//value
		auto arrayname = tokit->text;
		if (tokit->type != xctmp_token_t::TOKEN_ID){
			std::cerr << "error for syntax the array name not a id !" << tokit->text << std::endl;
			return -1;
		}
		//create for block
		env.chunks.push_back(xctmp_chunk_env_t());
		auto & chenv = env.chunks.back();
		chenv.itr = chunkit;
		auto v = _find_env_value(arrayname, -1, env);
		if (!v){
			std::cerr << "error array name ,not found in env define !" << tokit->text << std::endl;
			return -1;
		}
		if (!v->IsArray()){
			std::cerr << "error for array id , not a array type !" << tokit->text << std::endl;
			return -1;
		}
		chenv.loop_.itrname = itrname;
		chenv.loop_.arrayname = arrayname;
		chenv.loop_.idx = 0;
		chenv.loop_.total = v->Size();
		chenv.loop_.value = _find_env_value(arrayname, 0, env);
		if (!chenv.loop_.value){
			std::cerr << "not found the value index by 0 array name:"<< arrayname << std::endl;
			return -1;
		}
		if (chenv.loop_.idx >= chenv.loop_.total){
			//to end
			while ((++chunkit)->type != xctmp_chunk_t::CHUNK_BLOCK_END);
			--chunkit;
		}
		return 0;
	}
	else {
		std::cerr << "not support control flow tag yet: " << tag.text <<" in chunk: "<< chunkit->text << std::endl;
		return -1;
	}
}
static inline int
_render_end_block(xctmp_t* xc, std::string & output, xctmp_t::chunk_list_itr_t & chunkit, xctmp_env_t & env){
	if (env.chunks.empty()){
		std::cerr << "too much block end !" << std::endl;
		return -1;
	}
	else {
		auto & chk = env.chunks.back().itr; //current block
		auto & tag = *chk->toklist.begin();
		if (tag.text == "for"){ //for <v> in <v>(array)
			auto & for_env = env.chunks.back().loop_;
			++for_env.idx;
			if (for_env.idx < for_env.total){
				//get			
				auto v = _find_env_value(for_env.arrayname, for_env.idx, env);
				if (v){
					for_env.value = v;
				}
				else {
					std::cerr << "not found the for array name in env :" << for_env.itrname << std::endl;
					return -1;
				}
				chunkit = chk;//goto begin 
				chunkit++; //next one
				return 0;
			}
		}
		env.chunks.pop_back();
	}
	return 0;
}

int       
xctmp_render(xctmp_t * xc, std::string & output, const std::string & jsenv){
	auto it = xc->chunk_list.begin();
    xctmp_token_t result;
	xctmp_env_t env;
	//xctmp_control_flow_env_t	ctrl_env;
	int ret = env.global.loads(jsenv.c_str());
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
				std::cerr << "render error: " << result.value << " error expression: "<< it->text << std::endl;
				return -1;
			}
			else {
				std::cerr << "error eval: " << result.type << " value: " << result.value << std::endl;
			}
			break;
		case xctmp_chunk_t::CHUNK_COMMENT:
			break;
		case xctmp_chunk_t::CHUNK_BLOCK_CTL:			
			ret = _render_begin_block(xc, output, it, env);
			break;
		case xctmp_chunk_t::CHUNK_BLOCK_END:
			ret = _render_end_block(xc, output, it, env);
			break;
		default:
			ret = -1;
			break;
		}
		if (ret){
			std::cerr << "error block render : "<<it->text << std::endl;
			return ret;
		}
	}
	return 0;
}
void      
xctmp_destroy(xctmp_t * xc){
	delete xc;
}

//=====================================================================

bool xctmp_token_t::is_op() const {
	return priority() >= 100 && priority() <= 200;
}

bool xctmp_token_t::is_keywords() const{
	if (type == xctmp_token_t::TOKEN_ID &&
		(text == "if" ||
		text == "elif" ||
		text == "else" ||
		text == "for")){
		return true;
	}
	return false;
}

int xctmp_token_t::priority() const {
	//common	50
	//op 100 - 200	()
	//
	switch (type){
	case TOKEN_EQ:
	case TOKEN_LT:
	case TOKEN_GT:
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
bool	xctmp_token_t::operator < (const xctmp_token_t & op) const {
	if (type == op.type){
		if (type == xctmp_token_t::TOKEN_NUM && digit < op.digit){
			return true;
		}
		if (type == xctmp_token_t::TOKEN_STRING && value < op.value){
			return true;
		}
	}
	return false;
}
bool	xctmp_token_t::operator > (const xctmp_token_t & op) const {
	if (type == op.type){
		if (type == xctmp_token_t::TOKEN_NUM && digit > op.digit){
			return true;
		}
		if (type == xctmp_token_t::TOKEN_STRING && value > op.value){
			return true;
		}
	}
	return false;
}
bool xctmp_token_t::operator == (const xctmp_token_t & op) const {
	if (type == op.type){
		if (type == xctmp_token_t::TOKEN_NUM && digit == op.digit){
			return true;
		}
		if (type == xctmp_token_t::TOKEN_STRING && value == op.value){
			return true;
		}
	}
	return false;
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



