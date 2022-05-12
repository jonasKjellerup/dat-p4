#pragma once

/*
 * This header reimplements some type traits from the standard library
 * that are missing in avr-libc. The reimplementation only covers
 * the type traits that are used in the program.
 *
 * Should generally not be included directly but rather through
 * the <runtime/platform.hpp> header as it will determine if
 * the contents of this file is needed for the current target.
 */

namespace std {

    template<bool B>
    struct bool_constant {
        static constexpr bool value = B;
    };

    /*
     * Conditional type
     */

    template<bool B, typename T, typename F>
    struct conditional { using type = T; };

    template<typename T, typename F>
    struct conditional<false, T, F>{ using type = F; };

    /*
     * Reduced and modified version from the boost library
     * see https://stackoverflow.com/q/2910979
     */


    template <typename B, typename D>
    struct Host
    {
      constexpr operator B*() const;
      constexpr operator D*();
    };

    template <typename B, typename D>
    struct is_base_of
    {
      template <typename T>
      static constexpr bool_constant<true> check(D*, T);
      static constexpr bool_constant<false> check(B*, int);

      static constexpr bool value = decltype(check(Host<B,D>(), int()))::value;
    };

}
