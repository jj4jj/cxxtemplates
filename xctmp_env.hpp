#pragma  once
#include "json_doc.hpp"
#include <list>
#include <vector>
#include <string>

typedef std::string(* xctmp_ctrler_t)(const std::string &); //token list , chk list

struct xctmp_token_t {
	std::string		text; //static from tempalte buffer
	std::string		value;//dynamic value binding
	long long unsigned int	digit; //dyn
	enum xctmp_token_type {
		TOKEN_ERROR = -1,
		//empty
		TOKEN_NIL = 0,
		//id, num
		TOKEN_ID,
		TOKEN_NUM,
		TOKEN_STRING,
		//flow contrl 
		TOKEN_FILTER,
		TOKEN_CTRLER,

		//+-*/|() -----operator--------
		TOKEN_EQ,
		TOKEN_LT,
		TOKEN_GT,

		TOKEN_PLUS,
		TOKEN_MINUS,
		TOKEN_PRODUCT,
		TOKEN_DIV,
		TOKEN_FILT,
		///////////////////
		TOKEN_BRACKET_LEFT,
		TOKEN_BRACKET_RIGHT,
	} type;
	bool	is_op() const;
	bool	is_keywords() const ;
	int		priority() const;
	bool	operator == (const xctmp_token_t & op) const;
	bool	operator < (const xctmp_token_t & op) const;
	bool	operator > (const xctmp_token_t & op) const;

};

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
	xctmp_chunk_t();
	xctmp_chunk_t(const std::string & t);
	int     parse();
};

