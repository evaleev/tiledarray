/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  justus
 *  Department of Chemistry, Virginia Tech
 *
 *  vector_op.h
 *  Nov 17, 2013
 *
 */

#ifndef TILEDARRAY_MATH_VECTOR_OP_H__INCLUDED
#define TILEDARRAY_MATH_VECTOR_OP_H__INCLUDED

#ifndef TILEDARRAY_LOOP_UNWIND
#define TILEDARRAY_LOOP_UNWIND 8
#else

#if TILEDARRAY_LOOP_UNWIND != 1 || TILEDARRAY_LOOP_UNWIND != 2 || \
    TILEDARRAY_LOOP_UNWIND != 4 || TILEDARRAY_LOOP_UNWIND != 8 || \
    TILEDARRAY_LOOP_UNWIND != 16 || TILEDARRAY_LOOP_UNWIND != 32 || \
    TILEDARRAY_LOOP_UNWIND != 64 || TILEDARRAY_LOOP_UNWIND != 128 || \
    TILEDARRAY_LOOP_UNWIND != 256 || TILEDARRAY_LOOP_UNWIND != 512 || \
    TILEDARRAY_LOOP_UNWIND != 1024

#error TILEDARRAY_LOOP_UNWIND must be a power of 2 and less than or equal to 1024

#endif

#endif // TILEDARRAY_LOOP_UNWIND

#include <TiledArray/math/math.h>
#include <TiledArray/math/eigen.h>
#include <stdint.h>

// Add macro TILEDARRAY_FORCE_INLINE which does as the name implies.
#if (defined _MSC_VER) || (defined __INTEL_COMPILER)

#define TILEDARRAY_FORCE_INLINE __forceinline

#elif(__clang__)

#define TILEDARRAY_FORCE_INLINE __attribute__((always_inline)) inline

#elif defined(__GNUC__)

#if (__GNUC__ >= 4)
#define TILEDARRAY_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define TILEDARRAY_FORCE_INLINE inline
#endif // (__GNUC__ >= 4)

#else

#define TILEDARRAY_FORCE_INLINE inline

#endif


namespace TiledArray {
  namespace math {


    template <typename T, typename Op, typename Result>
    class BinderFirstHelper {
      const T& restrict t_;
      const Op& op_;

    public:
      typedef Result result_type;

      BinderFirstHelper(const T& restrict t, const Op& op) : t_(t), op_(op) { }

      template <typename Arg>
      TILEDARRAY_FORCE_INLINE result_type operator()(const Arg& restrict arg) const {
        return op_(t_, arg);
      }
    };


    template <typename T, typename Op>
    class BinderFirstHelper<T, Op, void> {
      const T& restrict t_;
      const Op& op_;

    public:
      typedef void result_type;

      BinderFirstHelper(const T& restrict t, const Op& op) : t_(t), op_(op) { }

      template <typename Result, typename Arg>
      TILEDARRAY_FORCE_INLINE void
      operator()(Result& restrict result, const Arg& restrict arg) const {
        return op_(result, t_, arg);
      }
    };

    template <typename T, typename Op>
    class BinderFirst : public BinderFirstHelper<T, Op, typename madness::detail::result_of<Op>::type> {
      typedef BinderFirstHelper<T, Op, typename madness::detail::result_of<Op>::type> BinderFirstHelper_;
    public:
      typedef typename BinderFirstHelper_::result_type result_type;

      BinderFirst(const T& restrict t, const Op& op) :
        BinderFirstHelper_(t, op)
      { }

    };

    template <typename T, typename Op>
    inline BinderFirst<T, Op> bind_first(const T& restrict t, const Op& op) {
      return BinderFirst<T, Op>(t, op);
    }

    template <typename T, typename Op, typename Result>
    class BinderSecondHelper {
      const T& restrict t_;
      const Op& op_;

    public:
      typedef Result result_type;

      BinderSecondHelper(const T& restrict t, const Op& op) : t_(t), op_(op) { }

      template <typename Arg>
      TILEDARRAY_FORCE_INLINE result_type operator()(const Arg& restrict arg) const {
        return op_(t_, arg);
      }
    };


    template <typename T, typename Op>
    class BinderSecondHelper<T, Op, void> {
      const T& restrict t_;
      const Op& op_;

    public:
      typedef void result_type;

      BinderSecondHelper(const T& restrict t, const Op& op) : t_(t), op_(op) { }

      template <typename Result, typename Arg>
      TILEDARRAY_FORCE_INLINE void operator()(Result& restrict result, const Arg& restrict arg) const {
        return op_(result, arg, t_);
      }
    };

    template <typename T, typename Op>
    class BinderSecond : public BinderSecondHelper<T, Op, typename madness::detail::result_of<Op>::type> {
      typedef BinderSecondHelper<T, Op, typename madness::detail::result_of<Op>::type> BinderSecondHelper_;
    public:
      typedef typename BinderSecondHelper_::result_type result_type;

      BinderSecond(const T& restrict t, const Op& op) :
        BinderSecondHelper_(t, op)
      { }

    };

    template <typename T, typename Op>
    inline BinderSecond<T, Op> bind_second(const T& restrict t, const Op& op) {
      return BinderSecond<T, Op>(t, op);
    }

    template <std::size_t N>
    struct VectorOpUnwind;

    /// Vector loop unwind helper class

    /// This object will unwind \c 1 step of a vector operation loop, and
    /// terminate the loop
    template <>
    struct VectorOpUnwind<0> {

      static const std::size_t offset = TILEDARRAY_LOOP_UNWIND - 1;

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      copy(const Arg* restrict const arg, Result* restrict const result) {
        result[offset] = arg[offset];
      }

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      scatter(const Arg* restrict const arg, Result* restrict const result, const std::size_t stride) {
        *result = arg[offset];
      }

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      gather(const Arg* restrict const arg, Result* restrict const result, const std::size_t stride) {
        *result = arg[offset];
      }

      template <typename Left, typename Right, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      binary(const Left* restrict const left, const Right* restrict const right,
          Result* restrict const result, const Op& op)
      {
        result[offset] = op(left[offset], right[offset]);
      }

      template <typename Arg, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      binary(const Arg* restrict const arg, Result* restrict const result, const Op& op) {
        op(result[offset], arg[offset]);
      }

      template <typename Arg, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      unary(const Arg* restrict const arg, Result* restrict const result, const Op& op) {
        result[offset] = op(arg[offset]);
      }


      template <typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      unary(Result* restrict const result, const Op& op) {
        op(result[offset]);
      }

      template <typename Left, typename Right, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      reduce(const Left* restrict const left, const Right* restrict const right,
          Result& restrict result, const Op& op)
      {
        op(result, left[offset], right[offset]);
      }

      template <typename Arg, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      reduce(const Arg* restrict const arg, Result& restrict result, const Op& op) {
        op(result, arg[offset]);
      }

    }; //  struct VectorOpUnwind

    /// Vector loop unwind helper class

    /// This object will unwind \c N steps of a vector operation loop.
    /// \tparam N The number of steps to unwind
    template <std::size_t N>
    struct VectorOpUnwind : public VectorOpUnwind<N - 1ul> {

      typedef VectorOpUnwind<N - 1ul> VectorOpUnwindN1;

      static const std::size_t offset = TILEDARRAY_LOOP_UNWIND - N - 1ul;

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      copy(const Arg* restrict const arg, Result* restrict const result) {
        result[offset] = arg[offset];
        VectorOpUnwindN1::copy(arg, result);
      }

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      scatter(const Arg* restrict const arg, Result* restrict const result, const std::size_t stride) {
        *result = arg[offset];
        VectorOpUnwindN1::scatter(arg, result + stride, stride);
      }

      template <typename Arg, typename Result>
      static TILEDARRAY_FORCE_INLINE void
      gather(const Arg* restrict const arg, Result* restrict const result, const std::size_t stride) {
        result[offset] = *arg;
        VectorOpUnwindN1::gather(arg + stride, result, stride);
      }

      template <typename Left, typename Right, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      binary(const Left* restrict const left, const Right* restrict const right,
          Result* restrict const result, const Op& op)
      {
        result[offset] = op(left[offset], right[offset]);
        VectorOpUnwindN1::binary(left, right, result, op);
      }

      template <typename Arg, typename Result, typename Op>
      static void TILEDARRAY_FORCE_INLINE
      binary(const Arg* restrict const arg, Result* restrict const result, const Op& op) {
        op(result[offset], arg[offset]);
        VectorOpUnwindN1::binary(arg, result, op);
      }

      template <typename Arg, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      unary(const Arg* restrict const arg, Result* restrict const result, const Op& op) {
        result[offset] = op(arg[offset]);
        VectorOpUnwindN1::unary(arg, result, op);
      }

      template <typename Result, typename Op>
      static void TILEDARRAY_FORCE_INLINE
      unary(Result* restrict const result, const Op& op) {
        op(result[offset]);
        VectorOpUnwindN1::unary(result, op);
      }

      template <typename Left, typename Right, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      reduce(const Left* restrict const left, const Right* restrict const right,
          Result& restrict result, const Op& op)
      {
        op(result, left[offset], right[offset]);
        VectorOpUnwindN1::reduce(left, right, result);
      }

      template <typename Arg, typename Result, typename Op>
      static TILEDARRAY_FORCE_INLINE void
      reduce(const Arg* restrict const arg, Result& restrict result, const Op& op) {
        op(result, arg[offset]);
        VectorOpUnwindN1::reduce(arg, result);
      }

    }; //  struct VectorOpUnwind

    typedef VectorOpUnwind<TILEDARRAY_LOOP_UNWIND - 1> VecOpUnwindN;
    typedef std::integral_constant<std::size_t, ~std::size_t(TILEDARRAY_LOOP_UNWIND - 1ul)> index_mask;


    template <typename Arg, typename Result, typename Op>
    void binary_vector_op(const std::size_t n, const Arg* restrict const arg,
        Result* restrict const result, const Op& op)
    {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += TILEDARRAY_LOOP_UNWIND) {
        Result* restrict const result_i = result + i;

        Result result_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(result_i, result_block);
        Arg arg_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(arg + i, arg_block);

        VecOpUnwindN::binary(arg_block, result_block, op);

        VecOpUnwindN::copy(result_block, result_i);

      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        Result result_block = result[i];
        const Arg arg_block = arg[i];

        op(result_block, arg_block);

        result[i] = result_block;
      }
    }

    template <typename Left, typename Right, typename Result, typename Op>
    void binary_vector_op(const std::size_t n, const Left* restrict const left,
        const Right* restrict const right, Result* restrict const result, const Op& op)
    {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += TILEDARRAY_LOOP_UNWIND) {

        Left left_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(left + i, left_block);
        Right right_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(right + i, right_block);

        Result result_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::binary(left_block, right_block, result_block, op);

        VecOpUnwindN::copy(result_block, result + i);
      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        const Left left_i = left[i];
        const Right right_i = right[i];

        const Result temp_i = op(left_i, right_i);

        result[i] = temp_i;

      }
    }

    template <typename Result, typename Op>
    void unary_vector_op(const std::size_t n, Result* restrict const result, const Op& op) {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += 8ul) {
        Result* restrict const result_i = result + i;

        Result result_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(result_i, result_block);

        VecOpUnwindN::unary(result_block, op);

        VecOpUnwindN::copy(result_block, result_i);

      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        Result temp_i = result[i];

        op(temp_i);

        result[i] = temp_i;

      }
    }

    template <typename Arg, typename Result, typename Op>
    void unary_vector_op(const std::size_t n, const Arg* restrict const arg,
        Result* restrict const result, const Op& op)
    {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += TILEDARRAY_LOOP_UNWIND) {

        Arg arg_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(arg + i, arg_block);

        Result result_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::unary(arg_block, result_block, op);

        VecOpUnwindN::copy(result_block, result + i);

      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        const Arg arg_i = arg[i];

        const Result temp_i = op(arg_i);

        result[i] = temp_i;
      }
    }


    template <typename Left, typename Right, typename Result, typename Op>
    void reduce_vector_op(const std::size_t n, const Left* restrict const left,
        const Right* restrict const right, Result& restrict result, const Op& op)
    {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += 8ul) {

        Left left_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(left + i, left_block);
        Right right_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(right + i, right_block);

        VecOpUnwindN::reduce(left_block, right_block, result, op);

      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        const Left left_block = left[i];
        const Right right_block = right[i];

        op(result, left_block, right_block);

      }
    }

    template <typename Arg, typename Result, typename Op>
    void reduce_vector_op(const std::size_t n, const Arg* restrict const arg,
        Result& restrict result, const Op& op)
    {
      std::size_t i = 0ul;

#if TILEDARRAY_LOOP_UNWIND > 1

      // Compute block iteration limit
      const std::size_t nx = n & index_mask::value;

      for(; i < nx; i += TILEDARRAY_LOOP_UNWIND) {

        Arg arg_block[TILEDARRAY_LOOP_UNWIND];
        VecOpUnwindN::copy(arg + i, arg_block);

        VecOpUnwindN::reduce(arg_block, result, op);

      }

#endif // TILEDARRAY_LOOP_UNWIND > 1

      for(; i < n; ++i) {

        const Arg arg_i = arg[i];

        op(result, arg_i);

      }
    }

  }  // namespace math
} // namespace TiledArray

#endif // TILEDARRAY_MATH_VECTOR_OP_H__INCLUDED
