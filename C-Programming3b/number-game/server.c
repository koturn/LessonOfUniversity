/*!
 * @brief サーバ側のプログラム
 *
 * @file   server.c
 * @author 2012-C3-group03
 * @version 1.0
 */

#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/numgame.h"
#include "lib/macro.h"
#include "lib/netutil.h"


// inlineが使えない処理系では、inline指定を消す
#if !defined(__GNUC__) && __STDC_VERSION__ < 199901L
#define inline
#endif

static int arg_parse(int argc, char *argv[], int *port_num);
static int make_socket(uint port_num);
inline static void show_usage(const char *prog_name);


/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の個数(プログラム名も含む)
 * @param [in] argv コマンドライン引数の配列
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  int sock_new;   /* ソケットのディスクリプタ */
  int port_num;   /* ポート番号 */

  /* 引数解析を行う */
  if (arg_parse(argc, argv, &port_num) == -1) {
    return EXIT_FAILURE;
  }
  /* クライアントとの接続を行い、クライアントのソケットディスクリプタを得る */
  if ((sock_new = make_socket(port_num)) == -1) {
    return EXIT_FAILURE;
  }
  return play_game_server(sock_new);   /* ゲームのメインループ */
}


/*!
 * @brief 引数解析を行う
 * @param [in]  argc     コマンドライン引数の個数(プログラム名も含む)
 * @param [in]  argv     コマンドライン引数の配列
 * @param [out] port_num ポート番号
 * @return 引数に異常がなければ0を、異常があれば-1を返す
 */
static int arg_parse(int argc, char *argv[], int *port_num) {
  switch (argc) {
    case 2:
      *port_num = convert_str2port(argv[1]);
      if (*port_num == -1) return -1;
    case 1:
      *port_num = DEFAULT_PORT;
      break;
    default:
      err_puts("引数は1つにしてください。");
      show_usage(argv[0]);
      return -1;
  }
  return 0;
}


/*!
 * @brief プログラムの使い方を表示する
 * @param [in] prog_name プログラム名
 */
static void show_usage(const char *prog_name) {
  puts   ("使い方:");
  println("    $ %s", prog_name);
  puts   ("  または");
  println("    $ %s [ポート番号]", prog_name);
  puts   ("\n終了方法:");
  puts   ("    ゲーム中に\"quit\"と入力する");
  puts   ("  または");
  puts   ("    Ctrl-C");
  puts   ("  で終了することができます。");
}


/*!
 * @brief ソケットを作成する
 * @param [in] port_num ポート番号
 * @return ソケットのディスクリプタ
 */
static int make_socket(uint port_num) {
  struct sockaddr_in me;
  int    sock;
  int    sock_new;

  memset((char *)&me, 0, sizeof(me));  /* aockaddr_in構造体変数meのゼロクリア */
  me.sin_family      = AF_INET;
  me.sin_addr.s_addr = htonl(INADDR_ANY);
  me.sin_port        = htons(port_num);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("socket");
    return -1;
  }

  if (bind(sock, (struct sockaddr *)&me, sizeof(me)) == -1){
    perror("bind");
    return -1;
  }

  listen(sock, 1);
  err_puts("successfully bound, now waiting.");
  sock_new = accept(sock, NULL, NULL);
  close(sock);

  return sock_new;
}
