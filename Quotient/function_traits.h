// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <functional>

namespace Quotient {

namespace _impl {
    template <typename>
    struct fn_traits {};
}

/// Determine traits of an arbitrary function/lambda/functor
/*!
 * Doesn't work with generic lambdas and function objects that have
 * operator() overloaded.
 * \sa
 * https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda#7943765
 */
template <typename T>
struct function_traits
    : public _impl::fn_traits<std::remove_reference_t<T>> {};

// Specialisation for a function
template <typename ReturnT, typename... ArgTs>
struct function_traits<ReturnT(ArgTs...)> {
    using return_type = ReturnT;
    using arg_types = std::tuple<ArgTs...>;
};

namespace _impl {
    template <typename>
    struct fn_object_traits;

    // Specialisation for a lambda function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_object_traits<ReturnT (ClassT::*)(ArgTs...)>
        : function_traits<ReturnT(ArgTs...)> {};

    // Specialisation for a const lambda function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_object_traits<ReturnT (ClassT::*)(ArgTs...) const>
        : function_traits<ReturnT(ArgTs...)> {};

    // Specialisation for function objects with (non-overloaded) operator()
    // (this includes non-generic lambdas)
    template <typename T>
    requires requires { &T::operator(); }
    struct fn_traits<T>
        : public fn_object_traits<decltype(&T::operator())> {};

    // Specialisation for a member function in a non-functor class
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<ReturnT (ClassT::*)(ArgTs...)>
        : function_traits<ReturnT(ClassT, ArgTs...)> {};

    // Specialisation for a const member function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<ReturnT (ClassT::*)(ArgTs...) const>
        : function_traits<ReturnT(const ClassT&, ArgTs...)> {};

    // Specialisation for a constref member function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<ReturnT (ClassT::*)(ArgTs...) const&>
        : function_traits<ReturnT(const ClassT&, ArgTs...)> {};

    // Specialisation for a prvalue member function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<ReturnT (ClassT::*)(ArgTs...) &&>
        : function_traits<ReturnT(ClassT&&, ArgTs...)> {};

    // Specialisation for a pointer-to-member
    template <typename ReturnT, typename ClassT>
    struct fn_traits<ReturnT ClassT::*>
        : function_traits<ReturnT&(ClassT)> {};

    // Specialisation for a const pointer-to-member
    template <typename ReturnT, typename ClassT>
    struct fn_traits<const ReturnT ClassT::*>
        : function_traits<const ReturnT&(ClassT)> {};
} // namespace _impl

template <typename FnT, int ArgN = 0>
using fn_arg_t =
    std::tuple_element_t<ArgN, typename function_traits<FnT>::arg_types>;

template <typename FnT>
constexpr auto fn_arg_count_v =
    std::tuple_size_v<typename function_traits<FnT>::arg_types>;

} // namespace Quotient
