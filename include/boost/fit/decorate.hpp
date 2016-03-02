/*=============================================================================
    Copyright (c) 2015 Paul Fultz II
    decorate.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef BOOST_FIT_GUARD_DECORATE_H
#define BOOST_FIT_GUARD_DECORATE_H

/// decorate
/// ========
/// 
/// Description
/// -----------
/// 
/// The `decorate` function adaptor helps create simple function decorators. 
/// 
/// A function adaptor takes a function and returns a new functions whereas a
/// decorator takes some parameters and returns a function adaptor. The
/// `decorate` function adaptor will return a decorator that returns a
/// function adaptor. Eventually, it will invoke the function with the user-
/// provided parameter and function.
/// 
/// Synopsis
/// --------
/// 
///     template<class F>
///     constexpr decorate_adaptor<F> decorate(F f);
/// 
/// Semantics
/// ---------
/// 
///     assert(decorate(f)(x)(g)(xs...) == f(x, g, xs...));
/// 
/// Requirements
/// ------------
/// 
/// F must be:
/// 
/// * [ConstCallable](concepts.md#constcallable)
/// * MoveConstructible
/// 
/// Example
/// -------
/// 
///     struct log_f
///     {
///         template<class F, class... Ts>
///         auto operator()(const std::string& message, F&& f, Ts&&... xs) const 
///             -> decltype(f(std::forward<Ts>(xs)...))
///         {
///             // Message to print out when the function is called
///             std::cout << message << std::endl;
///             // Call the function
///             return f(std::forward<Ts>(xs)...);
///         }
///     };
///     // The log decorator
///     BOOST_FIT_STATIC_FUNCTION(log) = boost::fit::decorate(log_f());
///     
///     struct sum_f
///     {
///         template<class T, class U>
///         T operator()(T x, U y) const
///         {
///             return x+y;
///         }
///     };
///     
///     BOOST_FIT_STATIC_FUNCTION(sum) = sum_f();
///     // Use the log decorator to print "Calling sum" when the function is called
///     assert(3 == log("Calling sum")(sum)(1, 2));
/// 

#include <boost/fit/reveal.hpp>
#include <boost/fit/detail/delegate.hpp>
#include <boost/fit/detail/move.hpp>
#include <boost/fit/detail/make.hpp>
#include <boost/fit/detail/callable_base.hpp>
#include <boost/fit/detail/static_const_var.hpp>
#include <boost/fit/detail/compressed_pair.hpp>

namespace boost { namespace fit { namespace detail {

template<class F, class T, class D>
struct decorator_invoke
: compressed_pair<F, T>, D
{
    typedef compressed_pair<F, T> base;

    template<class X1, class X2, class X3>
    constexpr decorator_invoke(X1&& x1, X2&& x2, X3&& x3)
    : base(BOOST_FIT_FORWARD(X1)(x1), BOOST_FIT_FORWARD(X2)(x2)), D(BOOST_FIT_FORWARD(X3)(x3))
    {}

    template<class... Ts>
    constexpr const F& base_function(Ts&&... xs) const
    {
        return this->first(xs...);
    }

    template<class... Ts>
    constexpr const T& get_data(Ts&&... xs) const
    {
        return this->second(xs...);
    }

    template<class... Ts>
    constexpr const D& get_decorator(Ts&&... xs) const
    {
        return always_ref(*this)(xs...);
    }

    BOOST_FIT_RETURNS_CLASS(decorator_invoke);

    struct decorator_invoke_failure
    {
        template<class Failure>
        struct apply
        {
            template<class... Ts>
            struct of
            : Failure::template of<const T&, const F&, Ts...>
            {};
        };
    };

    struct failure
    : failure_map<decorator_invoke_failure, D>
    {};

    template<class... Ts>
    constexpr BOOST_FIT_SFINAE_RESULT(const D&, id_<const T&>, id_<const F&>, id_<Ts>...) 
    operator()(Ts&&... xs) const BOOST_FIT_SFINAE_RETURNS
    (
        BOOST_FIT_MANGLE_CAST(const D&)(BOOST_FIT_CONST_THIS->get_decorator(xs...))(
            BOOST_FIT_MANGLE_CAST(const T&)(BOOST_FIT_CONST_THIS->get_data(xs...)),
            BOOST_FIT_MANGLE_CAST(const F&)(BOOST_FIT_CONST_THIS->base_function(xs...)),
            BOOST_FIT_FORWARD(Ts)(xs)...
        )
    );
};

template<class T, class D>
struct decoration
: compressed_pair<T, D>
{
    typedef compressed_pair<T, D> base;
    BOOST_FIT_INHERIT_CONSTRUCTOR(decoration, base)

    template<class... Ts>
    constexpr const D& get_decorator(Ts&&... xs) const
    {
        return this->second(xs...);
    }

    template<class... Ts>
    constexpr const T& get_data(Ts&&... xs) const
    {
        return this->first(xs...);
    }

    template<class F>
    constexpr decorator_invoke<detail::callable_base<F>, T, D> operator()(F f) const
    {
        return decorator_invoke<detail::callable_base<F>, T, D>(static_cast<F&&>(f), this->get_data(f), this->get_decorator(f));
    }
};

}

template<class F>
struct decorate_adaptor : detail::callable_base<F>
{
    typedef decorate_adaptor fit_rewritable1_tag;
    BOOST_FIT_INHERIT_CONSTRUCTOR(decorate_adaptor, detail::callable_base<F>)

    template<class... Ts>
    constexpr const detail::callable_base<F>& base_function(Ts&&... xs) const
    {
        return always_ref(*this)(xs...);
    }

    // TODO: Add predicate for constraints

    template<class T>
    constexpr detail::decoration<T, detail::callable_base<F>> operator()(T x) const
    {
        return detail::decoration<T, detail::callable_base<F>>(static_cast<T&&>(x), this->base_function(x));
    }

};

BOOST_FIT_DECLARE_STATIC_VAR(decorate, detail::make<decorate_adaptor>);

}} // namespace boost::fit

#endif