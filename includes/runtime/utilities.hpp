#pragma once

#include <runtime/platform.hpp>

// TODO find/write polyfill for avr-libc
#include <tuple>

template<typename, typename>
struct Prepend;

template<typename T, typename... Args>
struct Prepend<T, std::tuple<Args...>> {
    using type = std::tuple<T, Args...>;
};

template<template<typename>typename Predicate, template <typename ... > typename Target, typename...>
struct FilterInto;

template<template<typename>typename Predicate, template <typename ... > typename Target>
struct FilterInto<Predicate, Target> {
    using type = Target<>;
};

template<template<typename>typename Predicate, template <typename ... > typename Target, typename Head, typename...Tail>
struct FilterInto<Predicate, Target, Head, Tail...> {
    using type = typename std::conditional<
            Predicate<Head>::value,
            typename Prepend<Head, typename FilterInto<Predicate, Target, Tail...>::type>::type,
            typename FilterInto<Predicate, Target, Tail...>::type
            >::type;
};