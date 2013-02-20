/*!
 * @brief マクロのみを提供するヘッダファイル
 *
 * 強力なマクロを複数用意している。<br>
 * 関数のように扱うことができない(文法拡張的な)マクロについては、<br>
 * マクロ名の先頭に"$"を付けている。
 *
 * @author    Ueta Koji
 * @file      macro.h
 * @version   1.14.2.0
 * @attention C99規格のマクロもあるので、注意すること
 */


#ifndef MACRO_H_
#define MACRO_H_


#include <stdio.h>

#if !defined(_STDLIB_H_) && !defined(_INC_STDLIB)
/* *** <stdlib.h>中の関数のプロトタイプ宣言。リンクは自動的に行われる *** */
void *alloca (size_t size);             //!< alloca()関数のプロトタイプ
void *malloc (size_t size);             //!< malloc()関数のプロトタイプ
void *calloc (size_t n, size_t size);   //!< calloc()関数のプロトタイプ
void *realloc(void *ptr, size_t size);  //!< realloc()関数のプロトタイプ

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0  //!< システムが正常終了とする値
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1  //!< システムが異常終了とする値
#endif

#endif

#ifndef TRUE
#define TRUE  1  //!< TRUE
#endif
#ifndef FALSE
#define FALSE 0  //!< FALSE
#endif


/* ---------- よく使う型の型宣言(なお、このファイル内では用いていない) ---------- */
typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;




/* ------------------------------------------------------------
 * JavaのSystem.out.println()メソッドのような、改行付きプリント関数
 * C言語の文字列の結合を用いているので、
 * コンパイル後に、引数fmtの末尾に"\n"が追加される。
 * ダブルクオーテーション以外の文字列へのポインタを渡すことはできないが、
 * 書式指定文字列へのポインタを渡すことはあまりないと思われる。
 * マクロの可変長引数を利用している。
 * ------------------------------------------------------------ */
#define println(fmt, ...)         printf(fmt "\n", ##__VA_ARGS__)           //!< 改行付き printf()関数
#define fprintln(file, fmt, ...)  fprintf((file), fmt "\n", ##__VA_ARGS__)  //!< 改行付き fprintf()関数


/* ------------------------------------------------------------
 * 標準エラー出力用のプリント関数
 * マクロの可変長引数を利用している。
 * ------------------------------------------------------------ */
#define err_printf(fmt, ...)      fprintf(stderr, (fmt), ##__VA_ARGS__)     //!< 標準エラー出力へ出力する printf()関数
#define err_println(fmt, ...)     fprintln(stderr, fmt, ##__VA_ARGS__)      //!< 標準エラー出力へ出力する println()関数マクロ
#define err_puts(msg)             fputs(msg "\n", stderr)                   //!< 標準エラー出力へ出力する puts()関数


/* ------------------------------------------------------------
 * デバッグ用の出力関数
 * DEBUGマクロが定義されているときのみ有効。
 * DEBUGマクロが定義されていなければ、コンパイラの最適化により、
 * コード自体が除去される。
 *
 *   #ifdef DEBUG
 *   #define dbg_printf  printf
 *   #else
 *   #define dbg_printf  1 ? (void) 0 : printf
 *   #endif
 * のように、短く記述してもよかったが、
 *   3項演算子を悪用されないようにする
 * といった観点から、長い記述を用いた。
 * ------------------------------------------------------------ */
#ifdef DEBUG
#define dbg_printf(fmt, ...)         printf((fmt), ##__VA_ARGS__)           //!< デバッグ用 printf()関数
#define dbg_println(fmt, ...)        println(fmt, ##__VA_ARGS__)            //!< デバッグ用 println()関数マクロ
#define dbg_puts(msg)                puts(msg)                              //!< デバッグ用 puts()関数
#define dbg_fprintf(file, fmt, ...)  fprintf((file), (fmt), ##__VA_ARGS__)  //!< デバッグ用 fprintf()関数マクロ
#define dbg_fputs(msg, file)         fputs((msg), (file))                   //!< デバッグ用 fputs()関数
#define dbgerr_printf(fmt, ...)      err_printf((fmt), ##__VA_ARGS__)       //!< デバッグ用 err_printf()関数マクロ
#define dbgerr_println(fmt, ...)     err_println(fmt, ##__VA_ARGS__)        //!< デバッグ用 err_println()関数マクロ
#define dbgerr_puts(msg)             err_puts(msg)                          //!< デバッグ用 err_puts()関数マクロ

#else
// voidキャストしておかないと、警告レベルを上げたときにワーニングが出る。
#define dbg_printf(...)      (1 ? (void) 0 : printf(__VA_ARGS__))       //!< デバッグ用 printf()関数
#define dbg_println(...)     (1 ? (void) 0 : println(__VA_ARGS__))      //!< デバッグ用 println()関数マクロ
#define dbg_puts(...)        (1 ? (void) 0 : puts(__VA_ARGS__))         //!< デバッグ用 puts()関数
#define dbg_fprintf(...)     (1 ? (void) 0 : fprintf(__VA_ARGS__))      //!< デバッグ用 fprintf()関数マクロ
#define dbg_fputs(...)       (1 ? (void) 0 : fputs(__VA_ARGS__))        //!< デバッグ用 fputs()関数
#define dbgerr_printf(...)   (1 ? (void) 0 : err_printf(__VA_ARGS__))   //!< デバッグ用 err_printf()関数マクロ
#define dbgerr_println(...)  (1 ? (void) 0 : err_println(__VA_ARGS__))  //!< デバッグ用 err_println()関数マクロ
#define dbgerr_puts(...)     (1 ? (void) 0 : err_puts(__VA_ARGS__))     //!< デバッグ用 err_puts()関数マクロ

#endif


/*!
 * @brief 変数トレースマクロ
 *
 * 変数名と変数の値を表示する。
 * @param [in] var 表示したい変数
 * @param [in] fmt varの型の書式指定文字列
 */
#define TRACE(var, fmt)  printf(#var " = " fmt "\n", (var))


/*!
 * @brief 配列トレースマクロ
 *
 * 変数名と変数の値を表示する。<br>
 * gccで用いる場合は、式として扱われる。
 * @param [in] array 表示したい配列
 * @param [in] len   配列の要素数
 * @param [in] fmt   配列の要素の書式指定文字列
 */
#ifdef __GNUC__
#define DUMP_ARRAY(array, len, fmt)                                                    \
({                                                                                     \
  register unsigned int __tmp__loop_var__;                                             \
  for (__tmp__loop_var__ = 0; __tmp__loop_var__ < (len); __tmp__loop_var__++) {        \
    printf(#array "[%u] = " fmt "\n", __tmp__loop_var__, (array)[__tmp__loop_var__]);  \
  }                                                                                    \
})

#else
#define DUMP_ARRAY(array, len, fmt)                                                  \
{                                                                                    \
  register unsigned int __tmp_loop_var__;                                            \
  for (__tmp_loop_var__ = 0; __tmp_loop_var__ < (len); __tmp_loop_var__++) {         \
    printf(#array "[%u] = " fmt "\n", __tmp_loop_var__, (array)[__tmp_loop_var__]);  \
  }                                                                                  \
}

#endif


#ifdef DEBUG  /* ややこしいので、インデントして記述する */
  #if __STDC_VERSION__ >= 199901L
    //! 配列の1要素を、ファイル名、行番号、関数名と共に表示する
    #define ELEMENT_TRACE(array, idx, fmt)  \
      printf("%s at line %u in %s() : " #array "[%d] = " fmt "\n", __FILE__, __LINE__, __func__, (idx), (array)[idx])

    /*!
     * @brief デバッグ用変数トレース関数マクロ
     *
     * ファイル名と行番号、関数名を表示し、変数名と変数の値を表示する。
     * @param [in] var 表示したい変数
     * @param [in] fmt varの型の書式指定文字列
     */
    #define DBG_TRACE(var, fmt)  \
      printf("%s at line %u in %s() : " #var " = " fmt "\n", __FILE__, __LINE__, __func__, (var))

  #else   // [#if __STDC_VERSION__ < 199901L]  else
    /*!
     * @brief デバッグ用変数トレース関数マクロ
     *
     * ファイル名と行番号を表示し、変数名と変数の値を表示する。
     * @param [in] var 表示したい変数
     * @param [in] fmt varの型の書式指定文字列
     */
    #define DBG_TRACE(var, fmt)  \
      printf("%s at line %u : " #var " = " fmt "\n", __FILE__, __LINE__, (var))

    //! 配列の1要素を、ファイル名、行番号と共に表示する
    #define ELEMENT_TRACE(array, idx, fmt)  \
      printf("%s at line %u : " #array "[%d] = " fmt "\n", __FILE__, __LINE__, (idx), (array)[idx])
  #endif  // [#if __STDC_VERSION__ >= 199901L]  end

  #ifdef __GNUC__
    //! デバッグ用に配列の内容を全て表示する
    #define DBG_DUMP_ARRAY(array, len, fmt)                                       \
    ({                                                                            \
      register unsigned int __tmp_loop_var__;                                     \
      for (__tmp_loop_var__ = 0; __tmp_loop_var__ < (len); __tmp_loop_var__++) {  \
        ELEMENT_TRACE(array, __tmp_loop_var__, fmt);                              \
      }                                                                           \
    })
  #else   // [#ifdef __GNUC__]  else
    #define DBG_DUMP_ARRAY(array, len, fmt)                                       \
    {                                                                             \
      register unsigned int __tmp_loop_var__;                                     \
      for (__tmp_loop_var__ = 0; __tmp_loop_var__ < (len); __tmp_loop_var__++) {  \
        ELEMENT_TRACE(array, __tmp_loop_var__, fmt);                              \
      }                                                                           \
    }
  #endif  // [#ifdef __GNUC__]  end
#else   // [#ifdef DEBUG]  else
  //! デバッグ用変数トレース関数マクロ
  #define DBG_TRACE(...)       (1 ? (void) 0 : printf(""))
  //! デバッグ用に配列の内容を全て表示する
  #define DBG_DUMP_ARRAY(...)  (1 ? (void) 0 : printf(""))
#endif  // [#ifdef DEBUG]  end


/*!
 * @brief デバッグ用の式を記述できるマクロ
 *
 * 主に関数呼び出しに対して用いる。<br>
 * 使用例:
 *   @code  $debug printf("Hello World!\n");@endcode
 * DEBUGマクロが定義されていれば、このprintf()関数は有効となり、
 * 定義されていなければ、このprintf()関数は無効となる。<br>
 * ただし、
 *   @code  $debug num = abs(-200);@endcode
 * という呼び出し方はできず、
 *   @code  $debug(num = abs(-200);@endcode
 * と呼び出す必要がある。<br>
 *
 * また、以下のようなカンマ演算子を用いた記述も可能である。
 * @code
 *   $debug(
 *     printf("Hello "),
 *     printf("World!\n")
 *   );
 * @endcode
 */
#ifdef DEBUG
#define $debug
#else
#define $debug  1 ? (void) 0 :
#endif


/*!
 * @brief デバッグ用の式を記述できるマクロの安全版
 *
 * 主に関数呼び出しに対して用いる。<br>
 * $debugマクロよりも、カッコでくくられているため、安全である。<br>
 * 使用例:
 * @code
 *   $DEBUG(printf("Hello World!\n"));
 *   $DEBUG(num = 10);
 * @endcode
 * DEBUGマクロが定義されていれば、このprintf()関数は有効となり、
 * 定義されていなければ、このprintf()関数は無効となる。<br>
 * 可変引数を許容しているので、$debugマクロと同じく、
 * 以下の例のようにカンマ演算子を用いて複数のデバッグ式を記述できる。
 * @code
 *   $DEBUG(
 *     printf("Hello "),
 *     printf("World!\n")
 *   );
 * @endcode
 * $debugマクロでも同様の使い方はできるので、$DEBUG()マクロのアドバンテージはそんなに多くない。<br>
 */
#ifdef DEBUG
#define $DEBUG(...)  (__VA_ARGS__)
#else
#define $DEBUG(...)  (1 ? (void) 0 : (__VA_ARGS__))
#endif




/* ------------------------------------------------------------
 * 配列関係のマクロ
 * ------------------------------------------------------------ */
/*!
 * @brief 静的配列の要素数を得るマクロ
 * @param [in] array 1次元以上の静的配列
 * @return arrayの要素数
 */
#define LENGTH(array)          (sizeof(array) / sizeof((array)[0]))
//! LENGTH()関数マクロのエイリアス
#define length(array)           LENGTH(array)
//! 配列の終端アドレスを取得する
#define array_end(array)       ((array) + length(array))
//! 配列の最後の要素のアドレスを取得する
#define array_last(array)      ((array) + (length(array) - (size_t) 1))




/* ------------------------------------------------------------
 * 動的確保のラッパー関数マクロ
 * ------------------------------------------------------------ */
//! alloca()のラッパー関数マクロ
#define ALLOCA(type, n)        ((type *)alloca(sizeof(type) * (n)))
//! malloc()のラッパー関数マクロ
#define MALLOC(type, n)        ((type *)malloc(sizeof(type) * (n)))
//! calloc()のラッパー関数マクロ
#define CALLOC(type, n)        ((type *)calloc((n), sizeof(type)))
//! realloc()のラッパー関数マクロ
#define REALLOC(type, n, ptr)  ((type *)realloc((p), sizeof(type) * (n)))




/* ------------------------------------------------------------
 * 構造体関係のマクロ
 * ------------------------------------------------------------ */
#ifndef offsetof
//! 構造体メンバのオフセットを返す
#define offsetof(type, member)      ((size_t) &((type *) 0)->member)
#endif

//! 構造体のメンバサイズを返す
#define memsize(type, member)       sizeof(((type *) 0)->member)

//! 構造体の配列メンバの要素数を返す
#define memlength(type, mem_array)  length(((type *) 0)->mem_array)

//! 構造体メンバのオフセットから構造体のベースアドレスを逆算する
#define struct_base_offset(type, offset, mem_addr) \
  ((type *) ((char *) (mem_addr) - (offset)))

//! 構造体メンバのアドレスから構造体のベースアドレスを逆算する
#define struct_base(type, member, mem_addr) \
  struct_base_offset((type), offsetof(type, member), (mem_addr))




/*!
 * @brief 変数のビット数を得るマクロ
 * @param [in] var ビット数を調べたい変数、もしくは型名
 * @return varのビット数
 */
#define BITSIZE(var)   (sizeof(var) << 3)
//! BITSIZE()関数マクロのエイリアス
#define bitsize(var)   BITSIZE(var)




/* ------------------------------------------------------------
 * 値交換マクロ
 * 関数で書いた場合のAPIとの統合のため、全てポインタ渡しを想定している。
 * (C++ならば、参照渡しがあるので、この処置は不要であろう)
 * ------------------------------------------------------------ */
/*!
 * @brief どんな型にも用いることができるSWAPマクロ
 *
 * 型を指定する必要があるが、ポインタ型にも用いることが出来る。<br>
 * gccでコンパイルする場合は、丸括弧()で囲まれた複文を1つの式とみなす
 * という拡張機能を利用するように展開するが、<br>
 * gcc以外でコンパイルする場合は、文として完結しない
 *   @code  do { ... } while(0)@endcode
 * という形式を取った。(呼び出し側にセミコロンの付加を要求している)<br>
 * これにより、カッコを省略したif文で用いてもエラーは出なくなる。<br>
 * 交換速度は速い。
 *
 * @param [in]     type 交換する値の型名
 * @param [in,out] a    値1
 * @param [in,out] b    値2
 */
#ifndef __GNUC__
#define SWAP(type, a, b)                  \
do {                                      \
  register type __tmp_swap_var__ = *(a);  \
  *(a) = *(b);                            \
  *(b) = __tmp_swap_var__;                \
} while (0)


#else
#define SWAP(type, a, b)                  \
({                                        \
  register type __tmp_swap_var__ = *(a);  \
  *(a) = *(b);                            \
  *(b) = __tmp_swap_var__;                \
})


/*!
 * @brief コンパイル時に型を決定するSWAPマクロ
 *
 * gccの拡張機能(typeof演算子)により、変数の型を取得し、<br>
 * ポインタにも用いることが出来るSWAPマクロ。<br>
 * また、丸括弧()で囲まれた複文を1つの式とみなすという拡張機能も用いている。<br>
 * これにより、マクロ自体を式として扱うことができ、カッコを省略したif文や、<br>
 * カンマ演算子と共に用いることが可能になる。<br>
 * 交換速度は速い。
 *
 * C99でコンパイルする場合、
 *   @code  gcc -std=c99 foo.c@endcode
 * ではコンパイルできないので、
 *   @code  gcc -std=gnu99 foo.c@endcode
 * とコンパイルすること。
 *
 * @param [in,out] a 値1
 * @param [in,out] b 値2
 */
#define G_SWAP(a, b)                              \
({                                                \
  register typeof(*(a)) __tmp_swap_var__ = *(a);  \
  *(a) = *(b);                                    \
  *(b) = __tmp_swap_var__;                        \
})

#endif


/*!
 * @brief 整数用ビット演算高速SWAPマクロ
 *
 * 一時変数を用いずに、整数の値を交換する。<br>
 * ただし、同一の変数に用いてはならない(値が0になる)。<br>
 * また、ポインタ型のアドレス交換には用いることはできす、コンパイルエラーとなる<br>
 *
 * 使用場面を増やすため、ブロック文にせず、カンマ演算子で命令を連結させた。<br>
 * (そのため、無意味な値の返却があるが、値を用いない限りにおいては、
 * ブロック文で記述したときと生成される命令は変わらない)<br>
 *
 * 以下のようなトリッキーな書き方もあるが、<br>
 * 処理系によっては、未定義動作を引き起こす可能性がある。
 *   @code  #define WI_SWAP(a, b)  (a ^= b ^= a ^= b)@endcode
 *
 * @see SI_SWAP(a, b)
 * @see WI_SWAP(a, b)
 * @see WSI_SWAP(a, b)
 * @param [in,out] a 値1
 * @param [in,out] b 値2
 * @return 交換後のaの値(不本意な値の返却である)
 */
#define I_SWAP(a, b)  \
(                     \
  *(a) ^= *(b),       \
  *(b) ^= *(a),       \
  *(a) ^= *(b)        \
)


/*!
 * @brief 整数と浮動小数点数に用いることができるSWAPマクロ
 *
 * 一時変数を用いずに、整数/浮動小数点数の値を交換する。<br>
 * ただし、同一の変数に用いてはならない(値が0になる)。<br>
 * また、ポインタ型のアドレス交換には用いることはできず、コンパイルエラーとなる<br>
 *
 * 使用場面を増やすため、ブロック文にせず、カンマ演算子で命令を連結させた。<br>
 * (そのため、無意味な値の返却があるが、値を用いない限りにおいては、
 * ブロック文で記述したときと生成される命令は変わらない)<br>
 *
 * 以下のようなトリッキーな書き方もあるが、<br>
 * 処理系によっては、未定義動作を引き起こす可能性がある。
 *   @code  #define WF_SWAP(a, b)  (a += b -= a = b - a)@endcode
 *
 * @see SF_SWAP(a, b)
 * @see WF_SWAP(a, b)
 * @see WSF_SWAP(a, b)
 * @param [in,out] a 値1
 * @param [in,out] b 値2
 * @return 交換後のaの値(不本意な値の返却である)
 */
#define F_SWAP(a, b)    \
(                       \
  *(a) += *(b),         \
  *(b)  = *(a) - *(b),  \
  *(a) -= *(b)          \
)


/*!
 * @brief 整数用ビット演算高速SWAPマクロ : I_SWAP()のセーフティ版
 *
 * 一時変数を用いずに、整数/浮動小数点数の値を交換する。<br>
 * 同一の変数に用いても、値が0になることはない。<br>
 * 3項演算子による条件分岐を行っているので、アンセーフティ版より少し遅い。<br>
 * ポインタ型のアドレス交換には用いることはできない。<br>
 *
 * 使用場面を増やすため、ブロック文にせず、カンマ演算子で命令を連結させた。<br>
 * (そのため、無意味な値の返却があるが、値を用いない限りにおいては、
 * ブロック文で記述したときと生成される命令は変わらない)<br>
 *
 * &&演算子の性質を用いた以下のようなトリッキーな書き方もあるが、<br>
 * ワーニングレベルを上げると、計算結果を用いていないという趣旨のワーニング
 * ([-Wunused-value])が出る。
 *   @code  #define WSI_SWAP(a, b)  ((a != b) && (a ^= b ^= a ^= b))@endcode
 *
 * @see I_SWAP(a, b)
 * @see WI_SWAP(a, b)
 * @see WSI_SWAP(a, b)
 * @param [in,out] a 値1
 * @param [in,out] b 値2
 * @return 値を交換しなかった場合0を、交換した場合は1を返す。
 */
#define SI_SWAP(a, b)  \
(((a) == (b)) ? 0 :    \
(                      \
  *(a) ^= *(b),        \
  *(b) ^= *(a),        \
  *(a) ^= *(b),        \
  1                    \
))


/*!
 * @brief 整数と浮動小数点数に用いることができるSWAPマクロ : F_SWAP()のセーフティ版
 *
 * 一時変数を用いずに、整数/浮動小数点数の値を交換する。<br>
 * 同一の変数に用いても、値が0になることはない。<br>
 * 3項演算子による条件分岐を行っているので、アンセーフティ版より少し遅い。<br>
 * ポインタ型のアドレス交換には用いることはできない。<br>
 *
 * 使用場面を増やすため、ブロック文にせず、カンマ演算子で命令を連結させた。<br>
 * (そのため、無意味な値の返却があるが、値を用いない限りにおいては、
 * ブロック文で記述したときと生成される命令は変わらない)<br>
 *
 * &&演算子の性質を用いた以下のようなトリッキーな書き方もあるが、<br>
 * ワーニングレベルを上げると、計算結果を用いていないという趣旨のワーニング
 * ([-Wunused-value])が出る。
 *   @code  #define WSF_SWAP(a, b)  ((a != b) && (a += b -= (a = b - a)))@endcode
 *
 * @see F_SWAP(a, b)
 * @see WF_SWAP(a, b)
 * @see WSF_SWAP(a, b)
 * @param [in,out] a 値1
 * @param [in,out] b 値2
 * @return 値を交換しなかった場合0を、交換した場合は1を返す。
 */
#define SF_SWAP(a, b)   \
(((a) == (b)) ? 0 :     \
(                       \
  *(a) += *(b),         \
  *(b)  = *(a) - *(b),  \
  *(a) -= *(b),         \
  1                     \
))


/*!
 * @brief ワーニング([-Wsequence-point])が伴う I_SWAP()関数マクロ
 * @see I_SWAP(a, b)
 * @see SI_SWAP(a, b)
 * @see WSI_SWAP(a, b)
 */
#define WI_SWAP(a, b)   \
  (*(a) ^= *(b) ^= *(a) ^= *(b))

/*!
 * @brief ワーニング([-Wsequence-point])が伴う F_SWAP()関数マクロ
 * @see F_SWAP(a, b)
 * @see SF_SWAP(a, b)
 * @see WSF_SWAP(a, b)
 */
#define WF_SWAP(a, b)   \
  (*(a) += *(b) -= *(a) = *(b) - *(a))

/*!
 * @brief ワーニング([-Wsequence-point], [-Wunused-value])が伴う SI_SWAP()関数マクロ
 * @see I_SWAP(a, b)
 * @see SI_SWAP(a, b)
 * @see WI_SWAP(a, b)
 */
#define WSI_SWAP(a, b)  \
  (((a) != (b)) && (*(a) ^= *(b) ^= *(a) ^= *(b)))

/*!
 * @brief ワーニング([-Wsequence-point], [-Wunused-value])が伴う SF_SWAP()関数マクロ
 * @see F_SWAP(a, b)
 * @see SF_SWAP(a, b)
 * @see WF_SWAP(a, b)
 */
#define WSF_SWAP(a, b)  \
  (((a) != (b)) && (*(a) += *(b) -= *(a) = *(b) - *(a)))




/* ------------------------------------------------------------
 * ループ関連のマクロ
 * ------------------------------------------------------------ */
/*!
 * @brief 古典的なループマクロ
 *
 * 第2引数に式もしくは文を取るというトリッキーなマクロである。<br>
 * 使用例:
 * @code
 * $FOR_LOOP (10, {
 *     printf("Hello ");
 *     printf("World! %d\n", a);
 * }, a++)
 * @endcode
 *
 * 第2引数が式なら、次のように記述することも可能である。
 * @code
 * $FOR_LOOP (10, printf("a = %d\n"), a++)
 * @endcode
 *
 * また、第2引数の省略も可能である。
 * @code
 * $FOR_LOOP (10, , a++)
 * @endcode
 *
 * @see WHILE_LOOP(n, STATEMENT, ...)
 * @see DUFFS_LOOP(n, STATEMENT, ...)
 * @param [in]     n          ループ回数
 * @param [in,out] STATEMENT  ループ内で行う処理(省略可能。ただし、カンマを入れること)
 * @param [in,out] ...        次のステップ前に行う処理(省略可能)
 */
#define $FOR_LOOP(n, STATEMENT, ...)                                                 \
{                                                                                    \
  register unsigned int __tmp_loop_var__;                                            \
  for (__tmp_loop_var__ = n; __tmp_loop_var__; --__tmp_loop_var__, ##__VA_ARGS__) {  \
    STATEMENT;                                                                       \
  }                                                                                  \
}


/*!
 * @brief $FOR_LOOPマクロをwhile文で書いたもの
 *
 * 使い方は、FOR_LOOPマクロと同じである。
 * @see FOR_LOOP(n, STATEMENT, ...)
 * @see DUFFS_LOOP(n, STATEMENT, ...)
 * @param [in]     n          ループ回数
 * @param [in,out] STATEMENT  ループ内で行う処理(省略可能。ただし、カンマを入れること)
 * @param [in,out] ...        次のステップ前に行う処理(省略可能)
 */
#define $WHILE_LOOP(n, STATEMENT, ...)             \
{                                                  \
  register unsigned int __tmp_loop_var__ = n + 1;  \
  while (--__tmp_loop_var__) {                     \
    STATEMENT;                                     \
    __VA_ARGS__;                                   \
  }                                                \
}


/*!
 * @brief ダフのデバイス
 *
 * switch文のフォールスルーを利用した古典的なループのアンロール処理法。<br>
 * ループの条件分岐の比較回数を少なくし、高速化を目指している。<br>
 * 対象マシンによっては、高速化が期待できる。<br>
 * ただし、コンパイラの最適化の恩恵が受けにくくなるので、
 * 最適化オプションをつけてコンパイルする際には、注意が必要である。<br>
 * 8回分のアンロールとなっているが、もっと増やすことも可能である。<br>
 *
 * 使い方は、$FOR_LOOPマクロと同じであるが、コード量の増大が問題となるため、
 * 次のステップ前に行う処理は記述すべきでない。
 * @see FOR_LOOP(n, STATEMENT, ...)
 * @see WHILE_LOOP(n, STATEMENT, ...)
 * @param [in]     n          ループ回数
 * @param [in,out] STATEMENT  ループ内で行う処理(省略可能。ただし、カンマを入れること)
 * @param [in,out] ...        次のステップ前に行う処理(省略可能)
 */
#define $DUFFS_LOOP(n, STATEMENT, ...)             \
{                                                  \
  register int __tmp_loop_var__ = ((n) + 7) >> 3;  \
  switch ((n) & 7) {                               \
    case 0: do { STATEMENT; __VA_ARGS__;           \
    case 7:      STATEMENT; __VA_ARGS__;           \
    case 6:      STATEMENT; __VA_ARGS__;           \
    case 5:      STATEMENT; __VA_ARGS__;           \
    case 4:      STATEMENT; __VA_ARGS__;           \
    case 3:      STATEMENT; __VA_ARGS__;           \
    case 2:      STATEMENT; __VA_ARGS__;           \
    case 1:      STATEMENT; __VA_ARGS__;           \
            } while (--__tmp_loop_var__);          \
  }                                                \
}


//! ダフのデバイスの記法その2
#define $DUFFS_LOOP8(n, STATEMENT, ...)             \
{                                                   \
  register int __tmp_loop_var__ = (n);              \
  switch (__tmp_loop_var__ & 7) {                   \
    case 0: do { STATEMENT; __VA_ARGS__;            \
    case 7:      STATEMENT; __VA_ARGS__;            \
    case 6:      STATEMENT; __VA_ARGS__;            \
    case 5:      STATEMENT; __VA_ARGS__;            \
    case 4:      STATEMENT; __VA_ARGS__;            \
    case 3:      STATEMENT; __VA_ARGS__;            \
    case 2:      STATEMENT; __VA_ARGS__;            \
    case 1:      STATEMENT; __VA_ARGS__;            \
            } while ((__tmp_loop_var__ -= 8) > 0);  \
  }                                                 \
}


#if __STDC_VERSION__ >= 199901L

/*!
 * @brief C99規格でコンパイル可能なLOOPマクロ
 *
 * 内部でカウンタを用いることを想定していないfor文の代替として用いるとよい。
 *   @code  gcc -std=c99 foo.c@endcode
 * または、
 *   @code  gcc -std=gnu99 foo.c@endcode
 * とコンパイルすること。<br>
 *
 * 使用例:
 * @code
 * LOOP (100) {
 *   printf("Hello World!\n");
 * }
 * @endcode
 *
 * @param [in]     n   ループ回数
 * @param [in,out] ... 次のステップ前に行う処理(省略可能)
 */
#define $LOOP(n, ...)  \
  for (register unsigned int __loop_cnt__ = 0; __loop_cnt__ < (n); __loop_cnt__++, ##__VA_ARGS__)


/*!
 * @brief C99規格でコンパイル可能なREPEATマクロ
 *
 * 内部でカウンタを用いることを想定したfor文の代替として用いるとよい。
 *   @code  gcc -std=c99 foo.c@endcode
 * または、
 *   @code  gcc -std=gnu99 foo.c@endcode
 * とコンパイルすること。<br>
 *
 * 使用例:
 * @code
 * REPEAT (int, i, 100) {
 *   printf("i = %d\n", i);
 * }
 * @endcode
 *
 * @param [in]     type ループ制御変数の型
 * @param [in]     cnt  ループ制御変数名
 * @param [in]     n    ループ回数
 * @param [in,out] ...  次のステップ前に行う処理(省略可能)
 */
#define $REPEAT(type, cnt, n, ...)  \
  for (register type cnt = 0; cnt < (n); cnt++, ##__VA_ARGS__)

#endif

#define $UNTIL(...)   while (!(__VA_ARGS__))  //!< Rubyのようなuntil文の提供
#define until(...)    $UNTIL (__VA_ARGS__)    //!< $UNTIL()マクロのエイリアス

#define $UNLESS(...)  if (!(__VA_ARGS__))     //!< Rubyのようなunless文の提供
#define unless(...)   $UNLESS (__VA_ARGS__)   //!< $UNLESS()マクロのエイリアス


/* ------------------------------------------------------------
 * 数値計算用の関数マクロ
 * ------------------------------------------------------------ */
#define ABS(a)     ((a) > 0   ? (a) : -(a))  //!< 絶対値を得る関数マクロ
#define MAX(a, b)  ((a) > (b) ? (a) :  (b))  //!< 2数の内、大きい値を得る関数マクロ
#define MIN(a, b)  ((a) < (b) ? (a) :  (b))  //!< 2数の内、小さい値を得る関数マクロ
#define SQ(n)      ((n) * (n))               //!< 2乗した値を得る関数マクロ

//! MAX()関数マクロのエイリアス
#define MAX2(a, b)  MAX(a, b)
//! MIN()関数マクロのエイリアス
#define MIN2(a, b)  MIN(a, b)

//! 3数の内、大きい値を得る関数マクロ
#define MAX3(a, b, c)     ((a) > MAX((b), (c)) ? (a) : MAX((b), (c)))
//! 3数の内、小さい値を得る関数マクロ
#define MIN3(a, b, c)     ((a) < MIN((b), (c)) ? (a) : MIN((b), (c)))

//! 4数の内、大きい値を得る関数マクロ
#define MAX4(a, b, c, d)  ((a) > MAX3((b), (c), (d)) ? (a) : MAX3((b), (c), (d)))
//! 4数の内、小さい値を得る関数マクロ
#define MIN4(a, b, c, d)  ((a) < MIN3((b), (c), (d)) ? (a) : MIN3((b), (c), (d)))




/* ------------------------------------------------------------
 * ビット演算マクロ
 * ------------------------------------------------------------ */
//! 指定ビットの値(1 or 0)を得るマクロ
#define BITCHECK(a, b)  (((a) >> (b)) & 1)
//! BITCHECK()マクロのエイリアス
#define bitcheck(a, b)  BITCHECK(a, b)

//! 指定ビットに1をセットするマクロ
#define BITSET(a, b)    ((a) |= (1 << (b)))
//! BITSET()マクロのエイリアス
#define bitset(a, b)    BITSET(a, b)

//! 指定ビットに0をセットするマクロ
#define BITUNSET(a, b)  ((a) &= ~(1 << (b)))
//! BITUNSET()関数マクロのエイリアス
#define bitunset(a, b)  BITUNSET(a, b)




/* ------------------------------------------------------------
 * 整数型に対するマクロ
 * ------------------------------------------------------------ */
//! 指定された整数型が符号付きかどうかを判定する
#define Iissigned(itype)  ((itype) (-1) < 0)

//! 指定された整数型が2の補数表現か否かを判定する
#define Iistwos_cmpl(itype)                                      \
  (Iissigned(itype) &&                                           \
      ((itype) ((((itype) 1 << (bitsize(itype) - 2)) - 1) << 2)  \
    == (itype) (-4)))

//! 指定された整数型が1の補数表現か否かを判定する
#define Iisones_cmpl(itype)                                      \
  (Iissigned(itype) &&                                           \
      ((itype) ((((itype) 1 << (bitsize(itype) - 2)) - 1) << 2)  \
    == (itype) (-3)))

//! 指定された整数型が「符号ビット＋絶対値」か否かを判定する。
#define Iissign_abs(itype)                                       \
  (Iissigned(itype) &&                                           \
      ((itype) ((((itype) 1 << (bitsize(itype) - 2)) + 1) << 1)  \
     == (itype)(-2)))

//! 与えられた整数型の最小値を得る
#define Imin(itype)                                              \
  (Iissigned(itype)                                              \
    ? (itype) ((itype) 1 << (bitsize(itype) - 1)) : (itype) 0)

//! 与えられた整数型の最大値を得る
#define Imax(itype)                                              \
  (Iissigned(itype)                                              \
    ? (itype) ~((itype) 1 << (bitsize(itype) - 1)) : (itype) ~(itype) 0)

//! 整数同士の除算で、端数を切り捨てる(ただの割り算)
#define ifloor(dividend, divisor)  \
  ((dividend) / (divisor))

//! 整数同士の除算で、端数を切り上げる
#define iceil(dividend, divisor)  \
  (((dividend) + ((divisor) - 1)) / (divisor))

//! 整数同士の除算で、四捨五入する
#define iround(dividend, divisor)  \
  ((((dividend) << 1) + divisor) / ((divisor) << 1))



/* ------------------------------------------------------------
 * 文字コードに関するマクロ
 * ------------------------------------------------------------ */
//! Shift_JIS漢字第1バイトかどうかを判定する
#define is_sjis1(c)                                          \
  (((unsigned char)(c) > 0x80 && (unsigned char)(c) < 0xA0)  \
    || ((unsigned char)(c) > 0xCF && (unsigned char)(c) < 0xF0))

//! Shift_JIS漢字第2バイトかどうかを判定する
#define is_sjis2(c)  \
  ((unsigned char)(c) > 0x3F && (unsigned char)(c) < 0xFE && (unsigned char)(c) != 0x7F)




/* ------------------------------------------------------------
 * マクロの展開結果を表示するマクロ
 * ------------------------------------------------------------ */
//! 引数を文字列化するマクロ
#define M_STR(arg)           #arg
//! 文字列化するマクロへ中継するマクロ
#define MACRO_TOSTRING(arg)  M_STR(arg)
//! マクロの展開結果を出力するマクロ
#define PRINT_MACRO(MACRO) \
  printf("%s -> \"%s\"\n", #MACRO, MACRO_TOSTRING(MACRO))


#endif
