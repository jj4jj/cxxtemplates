#pragma once
#include <string>

typedef std::string(*xctmp_filter_t)(const std::string &);

struct    xctmp_t;
xctmp_t * xctmp_parse(const std::string & text);
int       xctmp_push_filter(xctmp_t * xc, const std::string & name, xctmp_filter_t filter);
int       xctmp_render(xctmp_t *, std::string & output, const std::string & xenv="{}");
void      xctmp_destroy(xctmp_t *);

