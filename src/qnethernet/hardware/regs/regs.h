// SPDX-FileCopyrightText: (c) 2026 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// regs.h defines some useful register-related utilities.
// This file is part of the imxrt1060-regs library.

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace qindesign {
namespace hardware {
namespace regs {

// Adds const to a member.
template <typename Layout, typename Member>
constexpr std::add_const_t<Member> Layout::* constify(
    Member Layout::* const m) {
  return m;
}

// Defines a register group having the given layout, expected size, and base
// address. While it's possible to just utilize sizeof(Layout) instead of also
// passing a size, having the size provides an extra check.
template <typename Layout, size_t LayoutSize, uintptr_t BaseAddr>
class RegGroup {
  static_assert(sizeof(Layout) == LayoutSize,
                "Bad size: Packing? Incorrect size?");
  static_assert((BaseAddr % alignof(Layout)) == 0, "Bad alignment");

 public:
  [[gnu::always_inline]]
  Layout* operator&() const { return reinterpret_cast<Layout*>(BaseAddr); }
  [[gnu::always_inline]]
  Layout* operator->() const { return operator&(); }
  [[gnu::always_inline]]
  Layout& operator*() const { return *operator&(); }
};

// Gets the basic shifted mask for the given parameters.
template <typename R, size_t Bits, unsigned int Shift>
[[gnu::always_inline]]
constexpr R shiftedMask() {
  // Number of bits in whole register.
  constexpr size_t kWholeRegBits = std::numeric_limits<R>::digits;

  // Parameter shape checks
  static_assert(std::is_integral_v<R> && std::is_unsigned_v<R>,
                "Type must be unsigned integral");
  static_assert(Bits != 0, "Bits shouldn't be zero");
  static_assert(Bits <= kWholeRegBits, "Bit count exceeds type width");
  static_assert(Shift < kWholeRegBits, "Shift exceeds type width");
  static_assert((Bits <= kWholeRegBits) && (Shift <= kWholeRegBits - Bits),
                "Value extends past type bounds");

  // Add -1 using R-bit modular arithmetic
  return static_cast<R>(
             (((Bits < std::numeric_limits<R>::digits) ? (R{1} << Bits)
                                                       : R{0}) +
              std::numeric_limits<R>::max())
             << Shift);
}

// Gets the basic shifted 32-bit mask for the given parameters.
template <size_t Bits, unsigned int Shift>
[[gnu::always_inline]]
constexpr uint32_t shiftedMask32() {
  return shiftedMask<uint32_t, Bits, Shift>();
}

// Gets the basic shifted 16-bit mask for the given parameters.
template <size_t Bits, unsigned int Shift>
[[gnu::always_inline]]
constexpr uint16_t shiftedMask16() {
  return shiftedMask<uint16_t, Bits, Shift>();
}

// Gets the basic shifted 8-bit mask for the given parameters.
template <size_t Bits, unsigned int Shift>
[[gnu::always_inline]]
constexpr uint8_t shiftedMask8() {
  return shiftedMask<uint8_t, Bits, Shift>();
}

// Reg defines an easier way to access parts of a register.
//
// The 'R' parameter is the unsigned type of each whole register, for example,
// uint32_t or uint16_t.
//
// There are two ways to write to the register:
// 1. Direct assignment
// 2. Read-modify-write: read -> AND(NOT(mask)) -> OR(value)
//
// For the second form, set '0' for the bits you'd like to keep and '1' for the
// bits that will be masked away. The first form is used when the mask is zero.
// The mask defaults to 1's for all the field's bit positions and 0's
// everywhere else.
//
// A direct assignment, when the mask is zero, is appropriate for things like
// "CLR" and "SET" registers where only the 1-assigned bits are set to
// something. Or, it could be used for write-1-to-clear bits, when all the other
// bits in the register are also SET or CLR or w1c.
//
// Note that caution should be observed when working with registers having
// fields marked with a zero 'AssignMask' if not all fields have a
// zero 'AssignMask'.
//
// The 'AssignSet' parameter guarantees certain bits are set.
template <typename R, uintptr_t Base, typename Layout,
          auto Member,          // Can be const or non-const
          size_t MemberOffset,  // If the member is an array, otherwise zero
          size_t Bits, unsigned int Shift,
          R AssignMask = shiftedMask<R, Bits, Shift>(),
          R AssignSet = 0,
          bool WriteOnly = false>
class Reg {
  using MemberExpr   = decltype(std::declval<Layout&>().*Member);
  using MemberType   = std::remove_reference_t<MemberExpr>;
  using RegType      = std::remove_extent_t<MemberType>;

  static constexpr bool kMemberIsWritable = !std::is_const_v<RegType>;
  static constexpr bool kMemberIsArray    = std::is_array_v<MemberType>;

  // Parameter shape checks
  static_assert(std::is_integral_v<R> && std::is_unsigned_v<R>,
                "Register type must be unsigned integral");
  static_assert(Bits != 0, "Bits shouldn't be zero");

  // Array checks
  static_assert(kMemberIsArray || (MemberOffset == 0),
                "Non-array members must use a zero offset");
  static_assert(!kMemberIsArray || (MemberOffset < std::extent_v<MemberType>),
                "Member offset out of range");

 public:
  // The register value type, with any const-volatile removed
  using RegValueType = std::remove_cv_t<RegType>;

  static_assert(std::is_same_v<RegValueType, R>,
                "Array element type must match register type");

  // Number of bits in whole register.
  static constexpr size_t kWholeRegBits = std::numeric_limits<R>::digits;

  // More parameter shape checks
  static_assert(Bits <= kWholeRegBits, "Bit count exceeds register width");
  static_assert(Shift < kWholeRegBits, "Shift exceeds register width");
  static_assert((Bits <= kWholeRegBits) && (Shift <= kWholeRegBits - Bits),
                "Register extends past register bounds");

  // The shift.
  static constexpr unsigned int kShift = Shift;

  // The number of consecutive bits.
  static constexpr size_t kBits = Bits;

  // The shifted mask.
  static constexpr R kMask = shiftedMask<R, Bits, Shift>();
  // static constexpr R kMask =
  //     static_cast<R>(std::make_signed_t<R>{-1}) >> (kWholeRegBits - Bits);
  // static constexpr R kMask = ((R{1} << Bits) - R{1});

  // Mask checks
  static_assert((AssignMask == 0) || ((AssignMask & kMask) == kMask),
                "Nonzero AssignMask must include the complete field mask");
  // static_assert((AssignSet & AssignMask) == 0);  // They should be disjoint
  static_assert((AssignSet & kMask) == 0);  // They should be "mostly" disjoint

  // Returns the masked and shifted version of the given value. This does not
  // include any bits from AssignSet.
  [[gnu::always_inline]]
  constexpr R operator()(const R val) const {
    return (val << Shift) & kMask;
  }

  // Given a whole register value, returns the extracted field. This is similar
  // to 'operator R()' and the opposite of assign().
  [[gnu::always_inline]]
  static constexpr R extract(const R val) {
    return (val & kMask) >> Shift;
  }

  // Given a whole register value, returns the new value with the field set to
  // the given field value. This is similar to assignment and the opposite
  // of extract().
  [[gnu::always_inline]]
  static constexpr R assign(const R r, const R field) {
    return (r & ~kMask) | ((field << Shift) & kMask);
  }

 private:
  [[gnu::always_inline]]
  static auto r() {
    auto& m = reinterpret_cast<Layout*>(Base)->*Member;
    if constexpr (kMemberIsArray) {
      return &m[MemberOffset];
    } else {
      static_assert(MemberOffset == 0, "Not an array");
      return &m;
    }
  }

 public:
  // Assigns the given value, after masking and shifting, to this register part.
  template <bool Writable = kMemberIsWritable,
            typename = std::enable_if_t<Writable>>
  [[gnu::always_inline]]
  const Reg& operator=(const R val) const {
    // Either directly assign or clear and then set the bits
    if constexpr ((AssignMask == 0) ||
                  ((Bits == kWholeRegBits) && (Shift == 0))) {
      *r() = static_cast<R>((*this)(val) | AssignSet);
    } else {
      *r() = static_cast<R>((*r() & ~AssignMask) | AssignSet | (*this)(val));
    }
    return *this;
  }

  // Gets the value of this register part. This returns the raw value not
  // masked or shifted.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  explicit operator R() const {
    return (*r() & kMask) >> Shift;
  }

  // Converts the register to an R. This is useful for when an explicit
  // conversion isn't desired. This calls operatorR().
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  R operator*() const {
    return static_cast<R>(*this);
  }

  // Performs the operation after masking and shifting the given value.
  //
  // This should not be used for w1c fields and the like.
  template <bool Writable = kMemberIsWritable,
            bool IsDirectAssign = (AssignMask == 0),
            bool Readable = !WriteOnly,
            typename = std::enable_if_t<Writable>,
            typename = std::enable_if_t<!IsDirectAssign>,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  const Reg& operator&=(const R val) const {
    const R curr = *r();
    *r() = static_cast<R>((curr & static_cast<R>(~AssignMask)) |
                          ((curr & kMask) & (*this)(val)) | AssignSet);
    return *this;
  }

  // Performs the operation after masking and shifting the given value.
  //
  // This should not be used for w1c fields and the like.
  template <bool Writable = kMemberIsWritable,
            bool IsDirectAssign = (AssignMask == 0),
            bool Readable = !WriteOnly,
            typename = std::enable_if_t<Writable>,
            typename = std::enable_if_t<!IsDirectAssign>,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  const Reg& operator|=(const R val) const {
    const R curr = *r();
    *r() = static_cast<R>((curr & static_cast<R>(~AssignMask)) |
                          (curr & kMask) | (*this)(val) | AssignSet);
    // *r() = static_cast<R>((*r() & (static_cast<R>(~AssignMask) | kMask)) |
    //                       (*this)(val) | AssignSet);
    return *this;
  }

  // Performs the operation after masking and shifting the given value.
  //
  // This should not be used for w1c fields and the like.
  template <bool Writable = kMemberIsWritable,
            bool IsDirectAssign = (AssignMask == 0),
            bool Readable = !WriteOnly,
            typename = std::enable_if_t<Writable>,
            typename = std::enable_if_t<!IsDirectAssign>,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  const Reg& operator^=(const R val) const {
    const R curr = *r();
    *r() = static_cast<R>((curr & static_cast<R>(~AssignMask)) |
                          ((curr & kMask) ^ (*this)(val)) | AssignSet);
    return *this;
  }

  // Performs the operation after masking and shifting the given value, and then
  // returns a value that isn't masked or shifted.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  R operator&(const R val) const {
    return ((*r() & (*this)(val)) & kMask) >> Shift;
  }

  // Performs the operation after masking and shifting the given value, and then
  // returns a value that isn't masked or shifted.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  R operator|(const R val) const {
    return ((*r() | (*this)(val)) & kMask) >> Shift;
  }

  // Performs the operation after masking and shifting the given value, and then
  // returns a value that isn't masked or shifted.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  R operator^(const R val) const {
    return ((*r() ^ (*this)(val)) & kMask) >> Shift;
  }

  // Performs the operation after masking and shifting the given value, and then
  // returns a value that isn't masked or shifted.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  R operator~() const {
    return ((~(*r())) & kMask) >> Shift;
  }

  // Tests if given value, after masking and shifting, is equal to this
  // register part.
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  bool operator==(const R val) const {
    return (*r() & kMask) == (*this)(val);
  }

  // Calls !operator==().
  template <bool Readable = !WriteOnly,
            typename = std::enable_if_t<Readable>>
  [[gnu::always_inline]]
  bool operator!=(const R val) const {
    return !(*this == val);
  }
};

// RegValue defines an easier way to define register-part values. It is
// represented by a mask and shift. It's useful for when a specific register
// value may be used for more than one register, and that value always has the
// same size and is always at the same position. It's also useful when the
// shifted value is used with regular R variables.
template <typename R, size_t Bits, unsigned int Shift>
class RegValue {
  // Parameter shape checks
  static_assert(std::is_integral_v<R> && std::is_unsigned_v<R>,
                "Register type must be unsigned integral");
  static_assert(Bits != 0, "Bits shouldn't be zero");

 public:
  // Number of bits in whole register.
  static constexpr size_t kWholeRegBits = std::numeric_limits<R>::digits;

  // More parameter shape checks
  static_assert(Bits <= kWholeRegBits, "Bit count exceeds register width");
  static_assert(Shift < kWholeRegBits, "Shift exceeds register width");
  static_assert((Bits <= kWholeRegBits) && (Shift <= kWholeRegBits - Bits),
                "Register extends past register bounds");

  // The shift.
  static constexpr unsigned int kShift = Shift;

  // The number of consecutive bits.
  static constexpr size_t kBits = Bits;

  // The shifted mask.
  static constexpr R kMask = shiftedMask<R, Bits, Shift>();

  // Returns the masked and shifted version of the given value.
  [[gnu::always_inline]]
  constexpr R operator()(const R val) const {
    return (val << Shift) & kMask;
  }

  // Given a whole register value, returns the extracted field. This is similar
  // to 'operator R()' and the opposite of assign().
  [[gnu::always_inline]]
  static constexpr R extract(const R val) {
    return (val & kMask) >> Shift;
  }

  // Given a whole register value, returns the new value with the field set to
  // the given field value. This is similar to assignment and the opposite
  // of extract().
  [[gnu::always_inline]]
  static constexpr R assign(const R r, const R field) {
    return (r & ~kMask) | ((field << Shift) & kMask);
  }
};

template <uintptr_t Base, typename Layout,
          auto Member,          // Can be const or non-const
          size_t MemberOffset,  // If the member is an array, otherwise zero
          size_t Bits, unsigned int Shift,
          auto AssignMask = shiftedMask32<Bits, Shift>(),
          uint32_t AssignSet = 0,
          bool WriteOnly = false>
using Reg32 = Reg<uint32_t, Base, Layout, Member, MemberOffset, Bits, Shift,
                  AssignMask, AssignSet, WriteOnly>;

template <size_t Bits, unsigned int Shift>
using RegValue32 = RegValue<uint32_t, Bits, Shift>;

template <uintptr_t Base, typename Layout,
          auto Member,          // Can be const or non-const
          size_t MemberOffset,  // If the member is an array, otherwise zero
          size_t Bits, unsigned int Shift,
          auto AssignMask = shiftedMask16<Bits, Shift>(),
          uint16_t AssignSet = 0,
          bool WriteOnly = false>
using Reg16 = Reg<uint16_t, Base, Layout, Member, MemberOffset, Bits, Shift,
                  AssignMask, AssignSet, WriteOnly>;

template <size_t Bits, unsigned int Shift>
using RegValue16 = RegValue<uint16_t, Bits, Shift>;

template <uintptr_t Base, typename Layout,
          auto Member,          // Can be const or non-const
          size_t MemberOffset,  // If the member is an array, otherwise zero
          size_t Bits, unsigned int Shift,
          auto AssignMask = shiftedMask8<Bits, Shift>(),
          uint8_t AssignSet = 0,
          bool WriteOnly = false>
using Reg8 = Reg<uint8_t, Base, Layout, Member, MemberOffset, Bits, Shift,
                 AssignMask, AssignSet, WriteOnly>;

template <size_t Bits, unsigned int Shift>
using RegValue8 = RegValue<uint8_t, Bits, Shift>;

// Generate unique "_reserved" field names
#define HARDWARE_REGS_CAT2(a, b) a##b
#define HARDWARE_REGS_CAT(a, b) HARDWARE_REGS_CAT2(a, b)
#define HARDWARE_REGS_LAYOUT_MEMBER_RESERVED \
  HARDWARE_REGS_CAT(_reserved, __LINE__)

}  // namespace regs
}  // namespace hardware
}  // namespace qindesign
