#pragma once
#include <string>
#include <unordered_map>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct xctmp_t;
struct xctmp_env_value_t {
	union {
		int64_t			i64;
		void		*	ptr;
	} uv_;
	std::string		str_;
	enum value_type {
		VALUE_ENV,
		VALUE_INT,
		VALUE_STR,
		VALUE_PTR,
	} type;
	xctmp_env_value_t(){
		type = VALUE_ENV;
		uv_.i64 = 0;
		uv_.ptr = nullptr;
	}
	xctmp_env_value_t(const std::string & s){
		type = VALUE_STR;
		str_ = s;
	}
	xctmp_env_value_t(void * p){
		type = VALUE_PTR;
		uv_.ptr = p;
	}
	xctmp_env_value_t(int64_t i){
		type = VALUE_INT;
		uv_.i64 = i;
	}

	template<typename T>
	T & Value(){
		return GetValue(T());
	}
	int64_t	& GetValue(int64_t){
		type = VALUE_INT;
		return uv_.i64;
	}
	std::string & GetValue(std::string){
		type = VALUE_STR;
		return str_;
	}
	void *		& GetValue(void *){
		type = VALUE_PTR;
		return uv_.ptr;
	}
};
struct xctmp_env_t : xctmp_env_value_t {
	typedef std::unordered_map<std::string, xctmp_env_t*>	dict_t;
	dict_t	dict;
	~xctmp_env_t(){
		for (auto it : dict){
			delete it.second;
		}
		dict.clear();
	}
	template<typename T>
	T & operator [](const std::string & key){
		xctmp_env_t * e = (dict)[key];
		if (!e){
			e = (dict)[key] = new xctmp_env_t();
		}
		return e->Value<T>();
	}
	std::string path(const std::string & pth){
		auto fnd = pth.find('.');//hello.hello
		if (pth.empty()){
			return this->str();
		}
		if (type != xctmp_env_value_t::VALUE_ENV){
			return "";//error type
		}
		std::string cp = pth.substr(0, fnd);
		auto it = dict.find(cp);
		if (it == dict.end()){
			return "";//not found
		}
		if (fnd == std::string::npos){
			return it->second->path("");
		}
		else {
			return it->second->path(pth.substr(fnd + 1));
		}
	}
	std::string str(){
		if (type == xctmp_env_value_t::VALUE_INT){
			return std::to_string(uv_.i64);
		}
		if (type == xctmp_env_value_t::VALUE_STR){
			return "\"" + str_ + "\"";//need to translate
		}
		if (type == xctmp_env_value_t::VALUE_PTR){
			char buff[20];
			snprintf(buff, sizeof(buff), "%p", uv_.ptr);
			return buff;
		}
		if (type == xctmp_env_value_t::VALUE_ENV){
			std::string v = "{";
			int i = 0;
			for (auto e : (dict)){
				if (i > 0){
					v += ",";
				}
				v += "\"" + e.first + "\": " + e.second->str();
				++i;
			}
			v += "}";
			return v;
		}
	}
};
template<>
xctmp_env_t & xctmp_env_t::operator [](const std::string & key){
	xctmp_env_t * e = (dict)[key];
	if (!e){
		e = (dict)[key] = new xctmp_env_t();
	}
	return *e;
}
template<>
xctmp_env_value_t & xctmp_env_t::operator [](const std::string & key){
	xctmp_env_t * e = (dict)[key];
	if (!e){
		e = (dict)[key] = new xctmp_env_t();
	}
	return *e;
}


xctmp_t * xctmp_parse(const std::string & text);
int       xctmp_render(xctmp_t *, std::string & output, const xctmp_env_t &);
void      xctmp_destroy(xctmp_t *);

