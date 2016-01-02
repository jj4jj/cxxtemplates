#pragma once
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/pointer.h"	//getter,setter by path(pointer)

typedef	typename rapidjson::Value			json_obj_t;
typedef typename rapidjson::Value::MemberIterator	json_obj_kv_itr_t;
typedef typename rapidjson::Value::ValueIterator	json_obj_list_itr_t;


class json_doc_t : public rapidjson::Document {
	#define MAX_CONF_BUFF_SIZE		(1024*1024)
	char *							parse_file_buffer;
public:
	json_doc_t(){
		parse_file_buffer = nullptr;
		SetObject();//root default is a object
	}
	~json_doc_t(){
		if (parse_file_buffer){
			delete parse_file_buffer;
		}
	}
public:
	int					parse_file(const char * file){
		FILE * fp = fopen(file, "r");
		if (!fp){
			return -1;
		}
		if (parse_file_buffer == nullptr && ( parse_file_buffer = new char [MAX_CONF_BUFF_SIZE])){
			return -2;
		}
		rapidjson::FileReadStream is(fp, parse_file_buffer, MAX_CONF_BUFF_SIZE);
		if (ParseStream(is).HasParseError()){
			rapidjson::ParseErrorCode e = rapidjson::Document::GetParseError();
			size_t o = GetErrorOffset();
			return -1;
		}
		return 0;
	}
	const	char *		pretty(std::string & str){
		str = "";
		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		Accept(writer);
		str = sb.GetString();
		return str.c_str();
	}
	int					loads(const char * buffer){
		rapidjson::StringStream	ss(buffer);
		if (ParseStream(ss).HasParseError()){
			rapidjson::ParseErrorCode e = GetParseError();
			size_t o = GetErrorOffset();
			return -1;
		}
		return 0;
	}
	const	char *		dumps(std::string & str){
		str = "";
		rapidjson::StringBuffer sb;
		rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
		Accept(writer);
		str = sb.GetString();
		return str.c_str();
	}
};
