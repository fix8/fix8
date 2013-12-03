//
// yield.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef reenter
# define reenter(c) CORO_REENTER(c)
#endif

#ifndef coro_yield
# define coro_yield CORO_YIELD
#endif

#ifndef coro_fork
# define coro_fork CORO_FORK
#endif
