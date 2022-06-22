#pragma once
#include <boost/preprocessor/cat.hpp>
#include <nonstd/scope.hpp>

#define eo_defer(...) auto BOOST_PP_CAT(_defer_, __COUNTER__) = nonstd::make_scope_exit(__VA_ARGS__)