/*!
 * @brief クライアント側のプログラム
 *
 * @file   client.c
 * @author 2012-C3-group03
 * @version 1.0
 */

#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/macro.h"
#include "lib/netutil.h"
#include "lib/calcgame.h"

#define PORT_NUM_LEN  6  //!< ポート番号を格納する配列の要素数
// inlineが使えない処理系では、inline指定を消す
#if !defined(__GNUC__) && __STDC_VERSION__ < 199901L
#define inline
#endif

static int arg_parse(int argc, char *argv[], char **hostname, char port_num_str[]);
static int make_socket(const char *hostname, const char *port_num_str);
inline static void show_usage(const char *prog_name);


/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の個数(プログラム名も含む)
 * @param [in] argv コマンドライン引数の配列
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  static char port_num_str[PORT_NUM_LEN];  /* ポート番号の文字列を収める配列 */
  int   sock;      /* ソケットのディスクリプタ */
  char *hostname;  /* ホストネーム */

  /* 引数解析を行う */
  if (arg_parse(argc, argv, &hostname, port_num_str) == -1) {
    return EXIT_FAILURE;
  }
  /* サーバとの接続を行い、クライアントのソケットディスクリプタを得る */
  if ((sock = make_socket(hostname, port_num_str)) == -1) {
    return EXIT_FAILURE;
  }
  return play_game_client(sock);  /* ゲームのメインループ */
}


/*!
 * @brief 引数解析を行う
 * @param [in]  argc         コマンドライン引数の個数(プログラム名も含む)
 * @param [in]  argv         コマンドライン引数の配列
 * @param [out] hostname     ホスト名
 * @param [out] port_num_str ポート番号(文字列)
 * @return 引数に異常がなければ0を、異常があれば-1を返す
 */
static int arg_parse(int argc, char *argv[], char **hostname, char port_num_str[]) {
  int port_num = DEFAULT_PORT;
  switch (argc) {
    case 3:  /* fall-through を利用し、case 2: へ */
      port_num = convert_str2port(argv[2]);
      if (port_num == -1) return -1;
    case 2:
      *hostname = argv[1];
      sprintf(port_num_str, "%d", port_num);
      break;
    default:
      err_puts("引数は2つにしてください。");
      show_usage(argv[0]);
      return -1;
  }
  return 0;
}


/*!
 * @brief プログラムの使い方を表示する
 * @param [in] prog_name プログラム名
 */
inline static void show_usage(const char *prog_name) {
  puts   ("使い方:");
  println("    %s [ホスト名]", prog_name);
  puts   ("  または");
  println("    %s [ホスト名] [ポート番号]", prog_name);
  puts   ("\n終了方法:");
  puts   ("    ゲーム中に\"quit\"と入力する");
  puts   ("  または");
  puts   ("    Ctrl-C");
  puts   ("  で終了することができます。");
}


/*!
 * @brief ソケットを作成する
 * @param [in] hostname     ホストネーム
 * @param [in] port_num_str ポート番号(文字列)
 * @return ソケットのディスクリプタ
 */
static int make_socket(const char *hostname, const char *port_num_str) {
  struct addrinfo hints;
  struct addrinfo *res;
  struct addrinfo *res_itr;  /* resのイテレータ */
  int sock = 0;              /* ソケットのディスクリプタ */

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(hostname, port_num_str, &hints, &res) != 0) {
    perror("getaadrinfo");
    return -1;
  }

  for (res_itr = res; res_itr != NULL; res_itr = res_itr->ai_next) {
    if ((sock = socket(res_itr->ai_family, res_itr->ai_socktype, res_itr->ai_protocol)) < 0) {
      continue;
    }
    if (connect(sock, res_itr->ai_addr, res_itr->ai_addrlen) != 0) {
      close(sock);
      continue;
    }
    break;
  }

  freeaddrinfo(res);
  return sock;
}
