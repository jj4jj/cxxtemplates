#pragma  once
#include <string>
#include <unordered_map>
#include <cassert>
#include <functional>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef std::string(* xctmp_func_t)(const std::string &);

struct xctmp_env_t {
    union {
        int64_t			i64;
        void		*	ptr;
    } uv_;
    std::string		    str_;
    xctmp_func_t        func_;
    enum value_type {
        VALUE_ENV,
        VALUE_INT,
        VALUE_STR,
        VALUE_FUNC,
    } type;
    typedef std::unordered_map<std::string, xctmp_env_t*>	dict_t;
    dict_t	dict;
    static xctmp_env_t  empty;
    ///////////////////////////////////////////////////////////////
    ~xctmp_env_t(){
        for (auto it : dict){
            delete it.second;
        }
        dict.clear();
    }
    ////////////////////////////////////////////////////////////////
    xctmp_env_t(){
        type = VALUE_ENV;
        uv_.i64 = 0;
        uv_.ptr = nullptr;
    }
    xctmp_env_t(const char * cs){
        type = VALUE_STR;
        str_ = cs;
    }
    xctmp_env_t(const std::string & s){
        type = VALUE_STR;
        str_ = s;
    }
    xctmp_env_t(const xctmp_func_t & f){
        type = VALUE_FUNC;
        func_ =  f;
    }
    xctmp_env_t(int64_t i){
        type = VALUE_INT;
        uv_.i64 = i;
    }

    xctmp_env_t & move(xctmp_env_t & rhs_){
        if (this != &rhs_){
            type = rhs_.type;
            str_ = rhs_.str();
            memcpy(&uv_, &rhs_.uv_, sizeof(uv_));
            //-----------------------------------
            dict = rhs_.dict;
            func_ = rhs_.func_;
            rhs_.dict.clear();
        }
        return *this;
    }
    xctmp_env_t & copy(xctmp_env_t & rhs_){
        if (this != &rhs_){
            type = rhs_.type;
            str_ = rhs_.str();
            memcpy(&uv_, &rhs_.uv_, sizeof(uv_));
            func_ = rhs_.func_;
            for (auto & it : rhs_.dict){
                this->operator[](it.first) = *(it.second);
            }
        }
        return *this;
    }
    xctmp_env_t(xctmp_env_t && rhs_){ // move syn
        move(rhs_);
    }
    xctmp_env_t & operator = (xctmp_env_t && rhs_){ // move syn
        return move(rhs_);
    }
    xctmp_env_t(xctmp_env_t & rhs_){ // move syn
        copy(rhs_);
    }
    xctmp_env_t & operator = (xctmp_env_t & rhs_){ // move syn
        return copy(rhs_);
    }
    ////////////////////////////////////////////////
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
    xctmp_func_t & GetValue(void *){
        type = VALUE_FUNC;
        return func_;
    }
    ////////////////////////////////////////////////////////////////
    xctmp_env_t & operator [](const std::string & key){
        type = VALUE_ENV;
        xctmp_env_t * e = (dict)[key];
        if (!e){e = (dict)[key] = new xctmp_env_t();}
        return *e;
    }
    const xctmp_env_t & operator [](const std::string & key) const {
        if (type != VALUE_ENV){
            return empty;
        }
        auto it = dict.find(key);
        if (it == dict.end()){
            return *it->second;
        }
        return empty;
    }
    xctmp_env_t & path(const std::string & pth){
        if (pth.empty()){
            return *this;
        }
        auto fnd = pth.find('.');//hello.hello
        std::string cp = pth.substr(0, fnd);
        xctmp_env_t & env = this->operator[](cp);
        if (fnd == std::string::npos){//hello
            return env;
        }
        else {//[hello].hello
            return env[pth.substr(fnd + 1)];
        }
    }
    const xctmp_env_t & path(const std::string & pth) const {
        if (pth.empty()){
            return *this;
        }
        auto fnd = pth.find('.');//hello.hello
        std::string cp = pth.substr(0, fnd);
        const xctmp_env_t & env = this->operator[](cp);
        if (fnd == std::string::npos){//hello
            return env;
        }
        else {//[hello].hello
            return env[pth.substr(fnd + 1)];
        }
    }
    std::string str() const {
        if (type == VALUE_INT){
            return std::to_string(uv_.i64);
        }
        if (type == VALUE_STR){
            return str_ ;
        }
        if (type == VALUE_FUNC){
            char buff[32];
#ifdef WIN32
            sprintf(buff, "function(%p)", func_);
#else
            snprintf(buff, sizeof(buff), "function(%p)", func_);
#endif
            return buff;
        }
        if (type == VALUE_ENV){
            std::string v = "{";
            int i = 0;
            for (auto e : (dict)){
                if (i > 0){
                    v += ",";
                }
                v += "\"" + e.first + "\": " + e.second->str();//esape
                ++i;
            }
            v += "}";
            return v;
        }
        return "";
    }
};