/*!
 * @brief サーバとクライアントに共通するユーティリティ関数を提供するヘッダファイル
 *
 * 実体は、netutil.cにある。
 *
 * @file   netutil.h
 * @author 2012-C3-group03
 * @version 1.0
 */

#ifndef NETUTIL_H
#define NETUTIL_H


#define MIN_PORT                 0  //!< ポート番号の最小値
#define MAX_PORT             65535  //!< ポート番号の最大値
#define MIN_WELL_KNOWN_PORT      0  //!< ウェルノウンポートの最小値
#define MAX_WELL_KNOWN_PORT   1023  //!< ウェルノウンポートの最大値
#define MIN_REGISTERED_PORT   1024  //!< 登録済みポート番号の最小値
#define MAX_REGISTERED_PORT  49151  //!< 登録済みポート番号の最大値
#define MIN_PRIVATE_PORT     49152  //!< 自由に使えるポート番号の最小値
#define MAX_PRIVATE_PORT     65535  //!< 自由に使えるポート番号の最大値

int convert_str2port(const char *str);


#endif
