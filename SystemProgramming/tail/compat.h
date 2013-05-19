/*!
 * @brief コンパイラや環境間の互換性のためのヘッダファイル
 *
 * Visual C++用のセキュアな関数と、<br>
 * その他のコンパイラで互換性を持たせるためのヘッダファイル。<br>
 * 完全な互換性はないので、注意すること。
 *
 * @author    koturn 0;
 * @date      2013 05/19
 * @file      compat.h
 * @version   0.1.1.0
 * @attention 安全ではない置き換えがあるので、注意すること
 */
#ifndef COMPAT_H
#define COMPAT_H


#ifdef _MSC_VER >= 1400  // Visual C++ (2005以降) のコンパイラならば


#define SCAN_S_ARG(arg)   (arg), (_countof(arg))


/* ------------------------------------------------------------
 * 入出力関係
 * ------------------------------------------------------------ */
#define printf(fmt, ...)           printf_s(fmt, ##__VA_ARGS__)
#define gets(dst)                  gets_s(dst, _countof(dst) - 1)
#define FOPEN(fp, filename, mode)  (fopen_s(&(fp), filename, mode), fp)


/* ------------------------------------------------------------
 * 文字列関係
 * ------------------------------------------------------------ */
#define strcpy(dst, src)          (strcpy_s(dst, _countof(dst), src), dst)
#define strcat(dst, src)          (strcat_s(dst, _countof(dst), src), dst)
#define strncat(dst, src, count)  (strncat(dst, _countof(dst), src, _TRUNCATE), dst)
#define strncpy(dst, src, count)  (strncpy(dst, _countof(dst), src, _TRUNCATE), dst)




#else
#define DUMMY_ERROR_NO  ((errno_t) 0)  //!< ダミーのエラーナンバー(0はエラー無しの意味)


/* ------------------------------------------------------------
 * 入出力関係
 * ------------------------------------------------------------ */
#define printf_s(fp, fmt, ...)        \
  printf(fp, fmt, ##__VA_ARGS__)

#define fprintf_s(fp, fmt, ...)       \
  fprintf(fp, fmt, ##__VA_ARGS__)

#define fopen_s(fpp, filename, mode)  \
  (*(fpp) = fopen(filename, mode), DUMMY_ERROR_NO)

#define gets_s(dst, dst_size)         \
  fgets(dst, dst_size, stdin)


/* ------------------------------------------------------------
 * scanf関係(可変引数部分でサイズ指定の必要がなければ、問題はない)
 * ------------------------------------------------------------ */
#define SCAN_S_ARG(arg)   (arg)

#define fscanf_s(fp, fmt, ...)       \
  fscanf(fp, fmt, ##__VA_ARGS__)

#define scanf_s(fmt, dst, dst_size)  \
  scanf(fmt, dst)

#define sscanf_s(str, fmt, ...)      \
  sscanf(str, fmt, ##__VA_ARGS__)


/* ------------------------------------------------------------
 * 文字列関係
 * ------------------------------------------------------------ */
#define memcpy_s(dst, dst_size, src, count)   \
  (memcpy(dst, src, count), DUMMY_ERROR_NO)

#define memmove_s(dst, dst_size, src, count)  \
  (memmove(dst, src, count), DUMMY_ERROR_NO)

#define strcat_s(dst, dst_size, src)          \
  (strcat(dst, src), DUMMY_ERROR_NO)

#define strcpy_s(dst, dst_size, src)          \
  (strcpy(dst, src), DUMMY_ERROR_NO)

#define strncat_s(dst, dst_size, src, count)  \
  (strncat(dst, src, count), DUMMY_ERROR_NO)

#define strncpy_s(dst, dst_size, src, count)  \
  (strncpy(dst, src), DUMMY_ERROR_NO)

#define strtok_s(str, demimiter, next_token)  \
  strtok(str, delimiter)

#define _strlwr_s(str, dst_size)              \
  (strlwr(str), DUMMY_FRROR_NO)

#define _strupr_s(str, dst_size)              \
  (strupr(str), DUMMY_FRROR_NO)

// ***
#define sprintf_s(dst, dst_size, fmt, ...)  \
  sprintf(dst, fmt, ##__VA_ARGS__)


#ifndef _CRT_RAND_S
#  define _CRT_RAND_S
#endif
#define rand_s(randomValue)  \
  (rand(randomValue), *(randomValue))

#endif



#if defined(WIN16) || defined(_WIN16) || defined(__WIN16) || defined(__WIN16__)   \
  || defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)  \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64) || defined(__WIN64__)
   // require <Windows.h>
#  define sleep(sec)  Sleep((sec) * 1000)  //!< Windwos向けのsleep()関数の代理
#endif


#endif
