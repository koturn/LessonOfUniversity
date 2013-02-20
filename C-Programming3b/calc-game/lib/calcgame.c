/*!
 * @brief 計算ゲームの各種関数を提供する
 *
 * @file   calcgame.c
 * @author 2012-C3-group03
 * @version 1.0
 */

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "macro.h"
#include "calcgame.h"

#define GOAL_POINT    5  //!< 勝利ポイント数
#define BUF_SIZE    256  //!< 入力を受け取るバッファのサイズ
#define RANGE_MAX   500  //!< 乱数の最大値
#define RANGE_MIN     1  //!< 乱数の最小値
#define INTERRUPT     0  //!< Ctrl-Cで終了したときに送る値


static void  judge(ulong input_time, ulong enemy_time, uint *my_cnt, uint *enemy_cnt);
static uchar check_end(uint my_cnt, uint enemy_cnt);
static ulong gettimeofday_sec(void);
static int   convert_str2int(const char *str);
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
  uint   my_cnt = 0, enemy_cnt = 0;

  sig_sock = sock;              /* シグナルハンドラが参照できるように、ソケットディスクリプタを設定 */
  signal(SIGINT, sig_handler);  /* シグナルハンドラの設定 */
  srand((uint)time(NULL));      /* 乱数のシードをセット */
  while (1) {
    int   ans = -1;
    int   a   = rrange(RANGE_MAX, RANGE_MIN);
    int   b   = rrange(RANGE_MAX, RANGE_MIN);
    ulong s_time, e_time, input_time;
    ulong enemy_time;
    puts("\n============================================================");

    s_time = gettimeofday_sec();    /* 解答開始時刻を取得 */
    do {
      memset(input_str, 0, sizeof(input_str));
      printf("%d + %d = ?\b", a, b);
      fgets(input_str, sizeof(input_str), stdin);
      e_time = gettimeofday_sec();  /* 解答終了時刻を取得 */

      chop_newline(input_str);
      if ((ans = convert_str2int(input_str)) == -1) continue;
      if (strncmp(input_str, "quit", 4) == 0)       return EXIT_SUCCESS;  /* "quit"と入力されたら終了 */
    } while (ans != a + b);
    input_time = e_time - s_time;
    printf("\nyour input time  = %ld ms\n", input_time);

    write(sock, &input_time, sizeof(input_time));  /* 自分の入力時間を送信 */

    /* 一旦フラッシュしておかないと、printfの内容が表示されないことがある */
    printf("wait for enemy input ...");
    fflush(stdout);
    /* 対戦相手の入力時間を受け取る */
    read(sock, &enemy_time, sizeof(enemy_time));
    if (enemy_time == INTERRUPT) {
      err_puts("\n対戦相手が強制終了しました");
      close(sock);
      return EXIT_FAILURE;
    }

    printf("\renemy input time = %ld ms\n", enemy_time);

    /* 対戦結果の表示 */
    judge(input_time, enemy_time, &my_cnt, &enemy_cnt);
    if (check_end(my_cnt, enemy_cnt)) {
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
  uint  my_cnt = 0, enemy_cnt = 0;

  sig_sock = sock;              /* シグナルハンドラが参照できるように、ソケットディスクリプタを設定 */
  signal(SIGINT, sig_handler);  /* シグナルハンドラの設定 */
  srand((uint)time(NULL));      /* 乱数のシードをセット */
  while (1) {
    int   ans = -1;
    int   a   = rrange(RANGE_MAX, RANGE_MIN);
    int   b   = rrange(RANGE_MAX, RANGE_MIN);
    ulong s_time, e_time, input_time;  /* 自分の入力開始･終了時刻と入力時間 */
    ulong enemy_time;                  /* 対戦相手の入力時間 */

    puts("\n============================================================");
    printf("wait for enemy input ...");
    fflush(stdout);

    /* 対戦相手の入力時間を受け取る */
    read(sock, &enemy_time, sizeof(enemy_time));
    if (enemy_time == INTERRUPT) {
      err_puts("\n対戦相手が強制終了しました");
      close(sock);
      return EXIT_FAILURE;
    }
    printf("\r                        \r");
    s_time = gettimeofday_sec();
    /* 問題解答部分 */
    do {
      memset(input_str, 0, sizeof(input_str));
      printf("%d + %d = ?\b", a, b);
      fgets(input_str, sizeof(input_str), stdin);
      e_time = gettimeofday_sec();  /* 解答終了時刻を取得 */

      chop_newline(input_str);
      if ((ans = convert_str2int(input_str)) == -1) continue;
      if (strncmp(input_str, "quit", 4) == 0)       return EXIT_SUCCESS;  /* "quit"と入力されたら終了 */
    } while (ans != a + b);
    input_time = e_time - s_time;  /* 入力時間の計測 */
    printf("\nyour input time  = %ld ms\n", input_time);
    write(sock, &input_time, sizeof(input_time));  /* 自分の入力時間を送信 */

    printf("enemy input time = %ld ms\n", enemy_time);

    /* 対戦結果の表示 */
    judge(input_time, enemy_time, &my_cnt, &enemy_cnt);
    if (check_end(my_cnt, enemy_cnt)) {
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


/*!
 * @brief 自分と相手のどちらの入力が速かったか、確認する
 *
 * 入力が速かったほうにポイントを加算する。
 * @param [in]  input_time 自分の入力時間
 * @param [in]  enemy_time 相手の入力時間
 * @param [out] my_cnt     自分のポイント
 * @param [out] enemy_cnt  相手のポイント
 */
static void judge(ulong input_time, ulong enemy_time, uint *my_cnt, uint *enemy_cnt) {
  if (input_time < enemy_time) {
    puts("You get a point!");
    (*my_cnt)++;
  } else if (input_time > enemy_time) {
    puts("Enemy get a point!");
    (*enemy_cnt)++;
  } else {
    puts("Draw game!");
  }

  printf("\nYour point    : %u\n", *my_cnt);
  printf("enemy's point : %u\n", *enemy_cnt);

}


/*!
 * @brief ゲームが終了しているか確認する。
 * @param [in] my_cnt    自分の正解数
 * @param [in] enemy_cnt 相手の正解数
 * @return ゲーム終了ならTRUEを、そうでないならFALSEを返す
 */
static uchar check_end(uint my_cnt, uint enemy_cnt) {
  if (my_cnt == GOAL_POINT) {
    puts("You win!!!!");
    return TRUE;
  } else if (enemy_cnt == GOAL_POINT) {
    puts("You lose!!!!");
    return TRUE;
  }
  return FALSE;
}


/*!
 * @brief 現在の時刻のミリ秒を返す
 *
 * time()関数は1秒単位で粒度が大きすぎる、<br>
 * clock()関数は、消費したCPU時間の計上のため、IOの待ち時間が計測できない<br>
 * といった理由のため、gettimeofday()関数を用いて、現座時刻のミリ秒を計測する。
 * @return 現在の時刻のミリ秒
 */
static ulong gettimeofday_sec(void) {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000 + t.tv_usec / 1000;
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
 * @return 変換した数値
 */
static int convert_str2int(const char *str) {
  char *check;
  int   num = strtol(str, &check, 10);  // char * -> long
  if (*check != '\0') {
    return -1;
  }
  if (num <= 0) {
    return -1;
  } else if (num == INT_MAX) {
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
  static const uchar FLAG = INTERRUPT;
  write(sig_sock, &FLAG, sizeof(FLAG));
  close(sig_sock);
  exit(signum);
}
