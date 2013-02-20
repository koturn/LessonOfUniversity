/*!
 * @brief サーバとクライアントに共通するユーティリティ関数を提供する
 *
 * @file   netutil.c
 * @author 2012-C3-group03
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "macro.h"
#include "netutil.h"


/*!
 * @brief 引数の文字列をポート番号の数値に変換する
 * @param [in] str ポート番号に変換する文字列
 * @return 変換に成功したならポート番号を、失敗したなら-1を返す
 */
int convert_str2port(const char *str) {
  char *check;
  int   num = strtol(str, &check, 10);  // char * -> long
  if (*check != '\0') {
    err_puts("ポート番号は数値で指定してください");
    return -1;
  }
  if (num < MIN_PORT || MAX_PORT < num) {
    err_println("ポート番号は %d~%d の範囲で指定してください", MIN_PRIVATE_PORT, MAX_PRIVATE_PORT);
    return -1;
  }
  if (MIN_WELL_KNOWN_PORT <= num && num <= MAX_WELL_KNOWN_PORT) {
    err_puts("ウェルノウンポートを指定しないでください。");
    err_println("ポート番号は %d~%d の範囲で指定してください", MIN_PRIVATE_PORT, MAX_PRIVATE_PORT);
    return -1;
  }
  if (MIN_REGISTERED_PORT <= num && num <= MAX_REGISTERED_PORT) {
    err_puts("注意: 登録済みポート番号が指定されています。");
    err_println("      ポート番号を %d~%d の範囲で指定することを推奨します", MIN_PRIVATE_PORT, MAX_PRIVATE_PORT);
  }
  return num;
}
