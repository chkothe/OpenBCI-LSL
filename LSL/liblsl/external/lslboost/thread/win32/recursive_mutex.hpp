#ifndef BOOST_RECURSIVE_MUTEX_WIN32_HPP
#define BOOST_RECURSIVE_MUTEX_WIN32_HPP

//  recursive_mutex.hpp
//
//  (C) Copyright 2006-7 Anthony Williams
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.lslboost.org/LICENSE_1_0.txt)


#include <lslboost/thread/win32/basic_recursive_mutex.hpp>
#include <lslboost/thread/exceptions.hpp>
#include <lslboost/thread/detail/delete.hpp>
#if defined BOOST_THREAD_PROVIDES_NESTED_LOCKS
#include <lslboost/thread/lock_types.hpp>
#endif

#include <lslboost/config/abi_prefix.hpp>

namespace lslboost
{
    class recursive_mutex:
        public ::lslboost::detail::basic_recursive_mutex
    {
    public:
        BOOST_THREAD_NO_COPYABLE(recursive_mutex)
        recursive_mutex()
        {
            ::lslboost::detail::basic_recursive_mutex::initialize();
        }
        ~recursive_mutex()
        {
            ::lslboost::detail::basic_recursive_mutex::destroy();
        }

#if defined BOOST_THREAD_PROVIDES_NESTED_LOCKS
        typedef unique_lock<recursive_mutex> scoped_lock;
        typedef detail::try_lock_wrapper<recursive_mutex> scoped_try_lock;
#endif
    };

    typedef recursive_mutex recursive_try_mutex;

    class recursive_timed_mutex:
        public ::lslboost::detail::basic_recursive_timed_mutex
    {
    public:
        BOOST_THREAD_NO_COPYABLE(recursive_timed_mutex)
        recursive_timed_mutex()
        {
            ::lslboost::detail::basic_recursive_timed_mutex::initialize();
        }
        ~recursive_timed_mutex()
        {
            ::lslboost::detail::basic_recursive_timed_mutex::destroy();
        }

#if defined BOOST_THREAD_PROVIDES_NESTED_LOCKS
        typedef unique_lock<recursive_timed_mutex> scoped_timed_lock;
        typedef detail::try_lock_wrapper<recursive_timed_mutex> scoped_try_lock;
        typedef scoped_timed_lock scoped_lock;
#endif
    };
}

#include <lslboost/config/abi_suffix.hpp>

#endif
