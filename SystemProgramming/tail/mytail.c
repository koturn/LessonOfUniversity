/*!
 * @brief tailコマンドの独自実装
 *
 * File encoding     : utf-8<br>
 * Newline Character : unix(LF)<br>
 *
 * ファイルの末尾を表示するUNIX/Linuxコマンド:tailを実装している。<br>
 * 基本的に外部に公開する予定は無いので、<br>
 * main()関数以外にはstatic修飾子をつけている<br>
 *
 * -fオプションを実現しているが、
 * OS間の差異を埋めるために、Windowsでコンパイルする場合、<br>
 * マクロを用いて、sleep()関数の代わりにSleep()関数を呼び出すようにしている。<br>
 *
 * コンパイルは単純に、
 *   <pre>    gcc mytail.c -o mytail</pre>
 * とすれば可能だが、
 *   <pre>    gcc -DDEBUG mytail.c -o mytail</pre>
 * とすると、デバッグ用のprint系のコードを付加する。<br>
 *
 * なお、このコメント形式からもわかるように、
 * doxygen形式のドキュメンテーションも行った。<br>
 * htmlドキュメント化したものは、作者(1G101097)のDropbox:
 *   <pre>    https://dl.dropbox.com/u/50294277/class/doxygen-mytail/index.html</pre>
 * に置いてある。
 *
 * @author Ueta Koji : 1G101097
 * @file   mytail.c
 */


#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) \
  || defined(WIN64) || defined(_WIN64) || defined(__WIN64) || defined(__WIN64__)
#include <Windows.h>
#define  sleep(msec)  Sleep(msec * 1000)  //!< Windwos向けのsleep()関数の代理

#else
#include <unistd.h>

#endif


#define LINE_LENGTH 1024  //!< 1行に読み込めるバイト数
#define LINE_NUM      10  //!< デフォルトで表示する末尾の行数
#define SLEEP_TIME     1  //!< -fオプション指定時にファイルを再度チェックする間隔(秒)

#ifndef TRUE
#define TRUE   1          //!< TRUEの値の定義
#endif

#ifndef FALSE
#define FALSE  0          //!< FALSEの値の定義
#endif

#define println(fmt, ...)         printf(fmt "\n", ##__VA_ARGS__)           //!< 改行付き printf()関数
#define fprintln(file, fmt, ...)  fprintf((file), fmt "\n", ##__VA_ARGS__)  //!< 改行付き fprintf()関数
#define err_println(fmt, ...)     fprintln(stderr, fmt, ##__VA_ARGS__)      //!< 標準エラー出力へ出力する println()関数マクロ
#define err_puts(msg)             fputs(msg "\n", stderr)                   //!< 標準エラー出力へ出力する puts()関数
#define MALLOC(type, n)           ((type *)malloc(sizeof(type) * (n)))      //!< malloc()のラッパーマクロ

/*!
 * @brief DEBUGマクロが指定されているときのみ、式を有効にするマクロ
 *
 * DEBUGマクロが指定されていないときは、コンパイラが命令を消去する。
 *   <pre>    $debug printf("Hello World!\n");</pre>
 *   <pre>    $debug(num = abs(-200));</pre>
 * のようにして用いる。
 */
#ifdef DEBUG
#define $debug
#else
#define $debug  1 ? (void) 0 :
#endif


typedef unsigned int  uint;
typedef unsigned char uchar;

static void   show_usage(const char *prog_name);
static int    arg_parse(int argc, char *argv[], uint *line_num, uchar *log_flag);
static int    convert_str2int(const char *str);
static char **alloc_ring_buf(uint line_num, uint len);
static void   free_ring_buf(char **ring_buf, uint line_num);
static uint   read_file(FILE *f, char **ring_buf, uint line_num, uint buf_size);
static void   show_ring_buf(char *const *ring_buf, uint line_num, uint rb_idx);
static long   update(FILE *f, long seek_pos);




/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の個数(プログラム名も含む)
 * @param [in] argv コマンドライン引数の配列
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  FILE   *f;                          /* 読み込むファイル名 */
  char   *filename = argv[argc - 1];  /* 一番最後の引数を読み込むファイル名とする */
  char  **ring_buf;                   /* リングバッファへのダブルポインタ */
  uint    line_num = LINE_NUM;        /* 表示する行数(デフォルトはLINE_NUMマクロで指定する数値) */
  uint    rb_idx;                     /* リングバッファの表示のときに用いるインデックス */
  uchar   log_flag = FALSE;           /* -fオプションが指定されたかどうかを確認するフラグ */
  long    seek_pos;                   /* ファイルのシーク位置を格納する。-fオプションの実現に用いる */

  /* 引数(オプション)解析 */
  if (arg_parse(argc, argv, &line_num, &log_flag) == -1) {
    return EXIT_FAILURE;
  }

  /* ファイル末尾の内容を格納する2次元配列を動的確保 */
  ring_buf = alloc_ring_buf(line_num, LINE_LENGTH);

  /* ファイルをオープンする */
  f = fopen(filename, "r");
  if (f == NULL) {
    err_println("ファイル: %s が開けません", filename);
    return EXIT_FAILURE;
  }

  /* ファイルを読み込み、リングバッファの開始インデックスを受け取る */
  rb_idx = read_file(f, ring_buf, line_num, LINE_LENGTH);
  /* リングバッファの内容を表示する(tailの表示部分) */
  show_ring_buf(ring_buf, line_num, rb_idx);
  /* -fオプションのために、読み込んだ位置を記憶する */
  seek_pos = ftell(f);
  /* 動的確保したリングバッファを解放する */
  free_ring_buf(ring_buf, line_num);

  /* -fオプションが指定されていなければ、終了する */
  if (!log_flag) return EXIT_SUCCESS;

  $debug println("========== -fオプションが表示する部分 ==========");
  while (TRUE) {
    f = fopen(filename, "r");
    if (f == NULL) {
      err_println("ファイル: %s が開けません", filename);
      return EXIT_FAILURE;
    }
    seek_pos = update(f, seek_pos);
    fclose(f);
    sleep(SLEEP_TIME);  /* 1秒停止する */
  }
  return EXIT_SUCCESS;
}


/*!
 * @brief プログラムの使い方を表示する。
 * @param [in] prog_name プログラム名
 */
static void show_usage(const char *prog_name) {
  puts   ("使い方");
  println("  %s [-options] prog_name", prog_name);
  puts   ("    [-options]は省略可能で、一番最後の引数の名前のファイルを読み込みます\n");

  puts   ("オプション");
  puts   ("  -f : ログ監視モードでファイルをオープンします");
  puts   ("  -n : 何行表示するかを決定します");
}


/*!
 * @brief 引数の解析を行う
 *
 * コマンドライン引数の解析(主にオプション解析)を行う・
 * @param [in]  argc     コマンドライン引数の個数(プログラム名も含む)
 * @param [in]  argv     コマンドライン引数の配列
 * @param [out] line_num tailコマンドで表示する行数
 * @param [out] log_flag ログ監視モードにするかどうかのフラグ
 * @return 正しい引数指定なら0を、そうでないなら-1を返す
 */
static int arg_parse(int argc, char *argv[], uint *line_num, uchar *log_flag) {
  char ch;  // オプション文字格納用変数

  if (argc < 2) {
    err_puts("読み込むファイル名を指定してください");
    show_usage(argv[0]);
    return -1;
  }

  while ((ch = getopt(argc, argv, "fhn:")) != -1) {
    switch (ch) {
      case 'f':  // ログ監視モードにする
        *log_flag = TRUE;
        break;
      case 'h':  // ヘルプを表示する
        show_usage(argv[0]);
        exit(EXIT_SUCCESS);
      case 'n':  // 表示行数を指定する
        *line_num = convert_str2int(optarg);
        break;
      default:
        err_puts("無効なオプションが指定されています");
        return -1;
    }
  }
  return 0;
}


/*!
 * @brief 引数の文字列を数値に変換する
 * @param [in] str 数値に変換する文字列
 * @return 変換した数値
 */
static int convert_str2int(const char *str) {
  char *check;
  int   num = strtol(str, &check, 10);
  if (*check != '\0') {
    fputs("文字列に数値以外がありました\n", stderr);
    exit(EXIT_FAILURE);
  }
  if (num <= 0) {
    fputs("0以下の値を指定しないでください\n", stderr);
    exit(EXIT_FAILURE);
  } else if (num == INT_MAX) {
    fputs("値が大きすぎます\n", stderr);
    exit(EXIT_FAILURE);
  }
  return num;
}


/*!
 * @brief 2次元配列の確保を行う
 *
 * 確保した2次元配列は、文字列格納用のリングバッファとして用いる。
 * @param [in] line_num リングバッファの要素数
 * @param [in] len      リングバッファ1要素当たりの最大文字列長
 * @return 動的確保したリングバッファの先頭アドレス
 */
static char **alloc_ring_buf(uint line_num, uint len) {
  uint i;
  char **array_top = MALLOC(char *, line_num);
  char **ring_buf  = array_top;

  if (array_top == NULL) {
    err_puts("メモリ確保に失敗しました");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < line_num; i++, ring_buf++) {
    *ring_buf = MALLOC(char, len);
    if (ring_buf == NULL) {
      err_puts("メモリ確保に失敗しました");
      exit(EXIT_FAILURE);
    }
  }
  return array_top;
}


/*!
 * @brief 動的確保したリングバッファのメモリを解放する
 *
 * alloc_ring_buf()関数と対にして用いる
 * @param [in] ring_buf 動的確保したしたリングバッファ
 * @param [in] line_num リングバッファの要素数
 */
static void free_ring_buf(char **ring_buf, uint line_num) {
  uint i;
  char **array_top = ring_buf;
  for (i = 0; i < line_num; i++, ring_buf++) {
    free(*ring_buf);
  }
  free(array_top);
}


/*!
 * @brief リングバッファにファイルを読み込む
 *
 * リングバッファに読み込み切らない場合は、NULLポインタを格納する。
 * @param [in]  f        読み込むファイルへのファイルポインタ
 * @param [out] ring_buf ファイルの内容を格納するリングバッファ
 * @param [in]  line_num リングバッファの要素数
 * @param [in]  buf_size リングバッファ1要素当たりのサイズ(byte)
 * @return リングバッファの開始インデックス
 */
static uint read_file(FILE *f, char **ring_buf, uint line_num, uint buf_size) {
  uint rb_idx   = 0;
  uint line_cnt = 0;
  while (fgets(ring_buf[rb_idx], buf_size, f) != NULL) {
    rb_idx = (rb_idx + 1) % line_num;
    line_cnt++;
  }

  // リングバッファが満タンになってないなら、開始インデックスとして0を返す
  if (line_cnt < line_num) {
    ring_buf[rb_idx] = NULL;  // リングバッファ内容出力時のセンチネルとなる
    return 0;
  } else {  // リングバッファが一周したとき
    return rb_idx;
  }
}


/*!
 * @brief リングバッファの内容を表示する
 * @param [in] ring_buf ファイルの内容を格納したリングバッファ
 * @param [in] line_num リングバッファの要素数
 * @param [in] rb_idx   リングバッファの開始インデックス
 */
static void show_ring_buf(char *const *ring_buf, uint line_num, uint rb_idx) {
  uint i;
  for (i = 0; i < line_num && ring_buf[rb_idx] != NULL; i++, rb_idx = (rb_idx + 1) % line_num) {
    $debug printf("%03d ", i + 1);  /* デバッグ時のみ行数も表示する */
    printf(ring_buf[rb_idx]);       /* リングバッファの文字列には、改行も含まれている */
  }
}


/*!
 * @brief ファイルの更新を監視する
 *
 * ファイルが追記されると、追記された部分を表示する。
 * @param [in,out] f        読み込むファイル
 * @param [in]     seek_pos 前回ファイルを読み込んだときのシーク位置
 * @return ファイルの終端位置
 */
static long update(FILE *f, long seek_pos) {
  static char buf[LINE_LENGTH];   /* 1行を読み込むバッファ */

  if (fseek(f, seek_pos, SEEK_SET) != 0) {
    err_puts("ファイルシークに失敗しました。");
  }
  /* 追記された部分を表示する */
  while (fgets(buf, sizeof(buf), f) != NULL) {
    printf(buf);
  }
  return ftell(f);  /* ファイル位置を返す。 */
}
