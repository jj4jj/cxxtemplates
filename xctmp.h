#pragma once
#include "xctmp_env.hpp"

struct xctmp_t;

xctmp_t * xctmp_parse(const std::string & text);
int       xctmp_render(xctmp_t *, std::string & output, xctmp_env_t &);
void      xctmp_destroy(xctmp_t *);

