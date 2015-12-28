#include "xctmp.h"


#if 0
struct xctmp_chunk_t {
	enum xctmp_chunk_type {
		CHUNK_TEXT,
		CHUNK_VAR,
		CHUNK_EXPR,
		CHUNK_COMMENT,
	} type;
	const std::string text;
};
struct xctmp_t {
	std::string					text;
	std::list<xctmp_chunk_t>	chunk_list;
};

xctmp_t * 
xctmp_parse(const std::string & text){
	xctmp_t * xc = new xctmp_t();
	//parse generate chunk list
	return xc;
}
static inline std::string 
_eval_expr(const std::string & expr, const xctmp_env_t & env){
	return "";
}
int       
xctmp_render(xctmp_t * xc, std::string & output, const xctmp_env_t & env){
	auto it = xc->chunk_list.begin();
	while (it != xc->chunk_list.end()){
		switch (it->type){
		case xctmp_chunk_t::CHUNK_TEXT:
			output.append(it->text);
			break;
		case xctmp_chunk_t::CHUNK_VAR:
			output.append(env.path(it->text));
			break;
		case xctmp_chunk_t::CHUNK_EXPR:
			output.append(_eval_expr(it->text, env));
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

#endif
