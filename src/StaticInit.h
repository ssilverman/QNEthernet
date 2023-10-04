// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// StaticInit.h helps avoid the Static Order Initialization Fiasco by using the
// Nifty Counter idiom. Briefly, it addresses using potentially uninitialized
// statically-initialized objects from other translation units.
//
// See:
// * https://en.cppreference.com/w/cpp/language/siof
// * https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Construct_On_First_Use
// * https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter
// This file is part of the QNEthernet library.

// To use for type `SomeType`:
//
// Header:
// 1. Include "StaticInit.h"
// 2. Inside the class declaration, add `friend class StaticInit<SomeType>;`
// 3. Add this after the class declaration: `STATIC_INIT_DECL(SomeType, name);`
//
// Source:
// 1. Add this: `STATIC_INIT_DEFN(SomeType, name);`

#ifndef QNETHERNET_STATICINIT_H_
#define QNETHERNET_STATICINIT_H_

#include <new>  // For placement new
#include <type_traits>

template <typename T>
struct StaticInit {
  StaticInit() {
    if (nifty_counter++ == 0) {
      new (&t_) T();  // Placement new
    }
  }

  ~StaticInit() {
    if (--nifty_counter == 0) {
      (&t_)->~T();
    }
  }

  static int nifty_counter;
  static std::aligned_storage_t<sizeof(T), alignof(T)> buf;

  T &t_ = reinterpret_cast<T &>(buf);
};

// Notes:
// 1. Static initializer for each translation unit
// 2. Can't currently declare more than one object of the same type
#define STATIC_INIT_DECL(Type, name)  \
  static StaticInit<Type> name##Init; \
  extern Type &name

// Notes:
// 1. Need to define the static class members
#define STATIC_INIT_DEFN(Type, name)                                  \
  template<> int StaticInit<Type>::nifty_counter = 0;                 \
  template<> decltype(StaticInit<Type>::buf) StaticInit<Type>::buf{}; \
  Type &name = name##Init.t_

#endif  // QNETHERNET_STATICINIT_H_
