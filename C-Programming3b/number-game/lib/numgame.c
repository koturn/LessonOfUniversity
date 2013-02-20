/*!
 * @brief 計算ゲームの各種関数を提供する
 *
 * @file   numgame.c
 * @author 2012-C3-group03
 * @version 1.0
 */

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "macro.h"
#include "numgame.h"

#define GOAL_POINT    5  //!< 勝利ポイント数
#define BUF_SIZE    256  //!< 入力を受け取るバッファのサイズ
#define RANGE_MAX    30  //!< 乱数の最大値
#define RANGE_MIN    21  //!< 乱数の最小値
#define INTERRUPT    -1 //!< Ctrl-Cで終了したときに送る値


static int   convert_str2int(const char *str, int a, int b);
static void  chop_newline(char *str);
static void  sig_handler(int signum);

#ifndef OPTIMIZE
static int rrange(int min, int max);

#else
//! 指定範囲の乱数値を得る
#define rrange(a, b)  \
  (a < b ? (rand() % (b - a + 1) + a) : (rand() % (a - b + 1) + b))

#endif


static int sig_sock;  //!< シグナルハンドラが利用するソケットディスクリプタ




/*!
 * @brief サーバ側のゲームのメインループ
 * @param [in] sock 送信先ソケットのディスクリプタ
 * @return 正常終了したなら0を、それ以外なら1を返す
 */
int play_game_server(int sock) {
  static char input_str[BUF_SIZE];  /* 入力文字を格納する配列 */
  int rest;

  sig_sock = sock;              /* シグナルハンドラが参照できるように、ソケットディスクリプタを設定 */
  signal(SIGINT, sig_handler);  /* シグナルハンドラの設定 */
  srand((uint)time(NULL));      /* 乱数のシードをセット */
  rest = rrange(RANGE_MIN, RANGE_MAX);

  write(sock, &rest, sizeof(rest));
  while (1) {
    int num = -1;
    int enemy_num;
    puts("\n============================================================");
    do {
      memset(input_str, 0, sizeof(input_str));
      printf("1 ~ 4 の範囲で取る数字を入力してください : 残り:%d\n", rest);
      printf("input>> ");
      fgets(input_str, sizeof(input_str), stdin);

      chop_newline(input_str);
      num = convert_str2int(input_str, 1, 4);
      if (rest - num < 0) {
        puts("残りが負の数になる数を指定しないでください\n");
        num = -1;
        continue;
      }
      if (strncmp(input_str, "quit", 4) == 0) return EXIT_SUCCESS;  /* "quit"と入力されたら終了 */
    } while (num == -1);
    rest -= num;
    printf("残り:%d\n\n", rest);
    write(sock, &num, sizeof(num));
    if (rest == 0) {
      puts("あなたの負けです");
      return EXIT_SUCCESS;
    }


    /* 一旦フラッシュしておかないと、printfの内容が表示されないことがある */
    printf("wait for enemy input ...");
    fflush(stdout);
    /* 対戦相手の入力を受け取る */
    read(sock, &enemy_num, sizeof(enemy_num));
    printf("\r                        \r");
    if (enemy_num == INTERRUPT) {
      err_puts("\n対戦相手が強制終了しました");
      close(sock);
      return EXIT_FAILURE;
    }
    printf("相手の入力数値>> %d\n", enemy_num);
    rest -= enemy_num;
    printf("残り:%d\n", rest);
    if (rest == 0) {
      puts("あなたの勝ちです");
      return EXIT_SUCCESS;
    }
  }
}


/*!
 * @brief クライアント側のゲームのメインループ
 * @param [in] sock 送信先ソケットのディスクリプタ
 * @return 正常終了したなら0を、それ以外なら1を返す
 */
int play_game_client(int sock) {
  static char input_str[BUF_SIZE];   /* 入力文字を格納する配列 */
  int rest;

  sig_sock = sock;              /* シグナルハンドラが参照できるように、ソケットディスクリプタを設定 */
  signal(SIGINT, sig_handler);  /* シグナルハンドラの設定 */
  srand((uint)time(NULL));      /* 乱数のシードをセット */
  read(sock, &rest, sizeof(rest));
  while (1) {
    int num = -1;
    int enemy_num;
    puts("\n============================================================");
    /* 一旦フラッシュしておかないと、printfの内容が表示されないことがある */
    printf("wait for enemy input ...");
    fflush(stdout);

    /* 対戦相手の入力を受け取る */
    read(sock, &enemy_num, sizeof(enemy_num));
    printf("\r                        \r");
    if (enemy_num == INTERRUPT) {
      err_puts("\n対戦相手が強制終了しました");
      close(sock);
      return EXIT_FAILURE;
    }
    printf("相手の入力数値>> %d\n", enemy_num);
    rest -= enemy_num;
    printf("残り:%d\n\n", rest);
    if (rest == 0) {
      puts("あなたの勝ちです");
      return EXIT_SUCCESS;
    }

    /* 入力部分 */
    do {
      memset(input_str, 0, sizeof(input_str));
      printf("1 ~ 4 の範囲で取る数字を入力してください : 残り%d\n", rest);
      printf("input>> ");
      fgets(input_str, sizeof(input_str), stdin);

      chop_newline(input_str);
      num = convert_str2int(input_str, 1, 4);
      if (rest - num < 0) {
        puts("残りが負の数になる数を指定しないでください\n");
        num = -1;
        continue;
      }
      if (strncmp(input_str, "quit", 4) == 0) return EXIT_SUCCESS;  /* "quit"と入力されたら終了 */
    } while (num == -1);
    rest -= num;
    printf("残り:%d\n", rest);
    write(sock, &num, sizeof(num));
    if (rest == 0) {
      puts("あなたの負けです");
      return EXIT_SUCCESS;
    }
  }
}


/*!
 * @brief テキストファイルから、出題文章を読み込む
 * @param [in]  filename  読み込むテキストファイル名
 * @param [out] questions 出題文章を読み込む2次元配列
 * @param [in]  len
 * @return 読み込んだ問題数+1
 */
uint read_question(const char *filename, char **questions, uint len) {
  FILE *f = fopen(filename, "r");
  register uint i;
  for (i = 0; fgets(questions[i], len, f) != NULL; i++);

  fclose(f);
  return i;
}


#ifndef OPTIMIZE
/*!
 * @brief 指定範囲内の乱数を得る
 *
 * 引数minの方がmaxより大きかった場合は、値を交換する
 * @deprecated インラインマクロが用意されています
 * @param [in] min 乱数の最小値
 * @param [in] max 乱数の最大値
 * @return 指定範囲内の乱数値
 */
static int rrange(int min, int max) {
  if (max < min) {
    SWAP(int, &max, &min);
  }
  return rand() % (max - min + 1) + min;
}
#endif


/*!
 * @brief 引数の文字列を数値に変換する。
 * @param [in] str 数値に変換する文字列
 * @param [in] min 入力できる最小値
 * @param [in] max 入力できる最大値
 * @return 変換した数値
 */
static int convert_str2int(const char *str, int min, int max) {
  char *check;
  int   num = strtol(str, &check, 10);  // char * -> long
  if (*check != '\0') {
    return -1;
  }
  if (num < min) {
    return -1;
  } else if (num > max) {
    return -1;
  }
  return num;
}


/*!
 * @brief 行末の改行文字を除去する
 * @param [out] str 改行文字を取り除く文字列
 */
static void chop_newline(char *str) {
  int n = strlen(str);
  if (str[n - 1] == '\n') {
    str[n - 1] = '\0';
  }
}


/*!
 * @brief シグナルハンドラ
 *
 * Ctrl-Cで強制終了したときに呼び出される。
 * @param [in] signum シグナル番号
 */
static void sig_handler(int signum) {
  static const int FLAG = INTERRUPT;
  write(sig_sock, &FLAG, sizeof(FLAG));
  close(sig_sock);
  exit(signum);
}
