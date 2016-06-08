#pragma once
#include <string>
#include <vector>

typedef std::string(*xctmp_filter_t)(const std::string &);

struct    xctmp_t;
xctmp_t * xctmp_parse(const std::string & text);
int       xctmp_push_filter(xctmp_t * xc, const std::string & name, xctmp_filter_t filter);
int       xctmp_push_n(xctmp_t * xc, const std::string & name, int64_t i);
int       xctmp_push_s(xctmp_t * xc, const std::string & name, const std::string & str);
int       xctmp_push_vn(xctmp_t * xc, const std::string & name, const std::vector<int64_t> &     vn);
int       xctmp_push_vs(xctmp_t * xc, const std::string & name, const std::vector<std::string> & vs);
void      xctmp_pop(xctmp_t * xc, const std::string & name);

int       xctmp_render(xctmp_t *, std::string & output, const std::string & xenv="{}");
void      xctmp_destroy(xctmp_t *);

