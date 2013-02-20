#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/data_handler.h"

#define DEFAULT_LEN       8192
#define DEFAULT_MERGE_NUM   30
#define DEFAULT_OUTPUT_FILENAME ("enshu3-out.txt")  // カッコでくくっておかないと、C言語の文字列の結合の危険性がある

static int  opt_parse(int argc, char *argv[], unsigned int *merge_num, char **in_filename, char **out_filename);
static int  convert_str2int(const char *str);
static void show_usage(const char *prog_name);
static void write_features(FILE *f, const feature *feature_datas, unsigned int len);




/*!
 * プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の個数(プログラム名も含む)
 * @param [in] argv コマンドライン引数の配列
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  static    data_fmt datas[DEFAULT_LEN];             /* csvデータを収める配列 */
  FILE     *in_fp;                                   /* 読み込むcsvファイルのファイルポインタ */
  FILE     *out_fp;                                  /* 書き込むファイルのファイルポインタ */
  char     *in_filename  = argv[argc - 1];           /* 読み込むcsvファイル名 */
  char     *out_filename = DEFAULT_OUTPUT_FILENAME;  /* 書き込むファイル名 */
  data_fmt *down_smpl_datas;                         /* ダウンサンプリングした後のデータ配列へのポインタ */
  feature  *feature_datas;                           /* ダウンサンプリングデータの特徴を収める配列へのポインタ */
  unsigned int len;                                  /* csvファイルの有効要素数 */
  unsigned int alloc_num;                            /* ダウンサンプリングデータの要素数 */
  unsigned int merge_num = DEFAULT_MERGE_NUM;        /* ダウンサンプリングで結合するデータの数 */

  // コマンドライン引数が無いとき、使い方を表示して終了
  if (argc < 2) {
    fputs("引数を指定してください\n", stderr);
    show_usage(argv[0]);
    return EXIT_FAILURE;
  }
  /* ----- オプション解析 ----- */
  if (opt_parse(argc, argv, &merge_num, &in_filename, &out_filename) != 0) {
    return EXIT_FAILURE;
  }

  /* ----- データの読み取り ----- */
  in_fp = fopen(in_filename, "r");  // 読み取るファイルをオープン
  if (in_fp == NULL) {  // ファイルがオープン出来ないとき、
    fprintf(stderr, "ファイル:%sが開けません\n", in_filename);
    return EXIT_FAILURE;
  }
  len = read_csv(in_fp, datas);  // ファイルを読み取り、有効データ数を取得
  fclose(in_fp);                 // 読み取ったファイルをクローズ


  /* ----- ダウンサンプリングデータと特徴データのメモリ確保 ----- */
  alloc_num       = len % merge_num == 0 ? (len / merge_num) : (len / merge_num + 1);
  down_smpl_datas = (data_fmt *)malloc(sizeof(data_fmt) * alloc_num);
  feature_datas   = (feature  *)malloc(sizeof(feature)  * alloc_num);
  if (down_smpl_datas == NULL || feature_datas == NULL) {
    fputs("メモリ確保に失敗しました\n", stderr);
    return EXIT_FAILURE;
  }


  /* -----  ダウンサンプリングと特徴データの抽出 ----- */
  down_sample(down_smpl_datas, datas, len, merge_num);
  derive_features(feature_datas, down_smpl_datas, alloc_num);


  /* ----- データの書き込み ----- */
  out_fp = fopen(out_filename, "w");  // 出力ファイルをオープン
  if (out_fp == NULL) {  // ファイルがオープン出来ないとき、
    fprintf(stderr, "ファイル:%sに書き込むことが出来ませんでした\n", out_filename);
    return EXIT_FAILURE;
  }
  write_features(out_fp, feature_datas, alloc_num);  // ファイルに書き込む


  // この後すぐにプログラムを終了するので、
  // 明示的に解放しなくともよいが、お行儀よく解放しておく。
  free(down_smpl_datas);  // ダウンサンプリングデータ領域の解放
  free(feature_datas);    // 特徴データ領域の解放
  return EXIT_SUCCESS;    // 正常終了
}


/*!
 * オプションを解析する
 * @param [in]  argc         コマンドライン引数の個数(プログラム名も含む)
 * @param [in]  argv         コマンドライン引数の配列
 * @param [out] merge_num    ダウンサンプリングでまとめる数
 * @param [out] in_filename  読み込むファイル名
 * @param [out] out_filename 出力ファイル名
 * @return 正常に解析出来たならば0を、プログラムを終了させるときは-1を返す
 */
static int opt_parse(int argc, char *argv[], unsigned int *merge_num, char **in_filename, char **out_filename) {
  char ch;  // オプション文字格納用変数
  while ((ch = getopt(argc, argv, "f:hm:o:")) != -1) {
    switch (ch) {
      case 'f':  // 入力ファイル名を指定する
        *in_filename = optarg;
        break;
      case 'h':  // ヘルプを表示する
        show_usage(argv[0]);
        exit(EXIT_SUCCESS);   // ヘルプは正常終了コードをシステムに返す
      case 'm':  // ダウンサンプリングでまとめる数を指定
        *merge_num = convert_str2int(optarg);
        break;
      case 'o':  // 出力ファイル名を指定する
        *out_filename = optarg;
        break;
      default:   // 存在しないオプションが指定されたとき
        fputs("無効なオプションが指定されています\n", stderr);
        return -1;
    }
  }
  return 0;
}


/*!
 * 引数の文字列を数値に変換する。
 * @param [in] str 数値に変換する文字列
 * @return 変換した数値
 */
static int convert_str2int(const char *str) {
  char *check;
  int   num = strtol(str, &check, 10);  // char * -> long
  if (*check != '\0') {
    fputs("文字列に数値以外がありました\n", stderr);
    exit(EXIT_FAILURE);
  }
  if (num <= 0) {
    fputs("ダウンサンプリングの要素数に0以下の値を指定しないでください\n", stderr);
    exit(EXIT_FAILURE);
  } else if (num == INT_MAX) {
    fputs("ダウンサンプリングの要素数の値が大きすぎます\n", stderr);
    exit(EXIT_FAILURE);
  }
  return num;
}


/*!
 * プログラムの使い方を表示する
 * @param [in] prog_name プログラム名
 */
static void show_usage(const char *prog_name) {
  puts  ("使い方:");
  printf("    %s [-options] filename\n", prog_name);
  puts  ("  または");
  printf("    %s [-options] -f filename [-options]\n\n", prog_name);

  puts("オプション:");
  puts("  -f : 入力csvファイル名を指定します");
  puts("  -h : 使い方を表示します");
  puts("  -m : ダウンサンプリングでまとめる要素数を指定します");
  puts("  -o : 出力ファイル名を指定します\n");

  puts("使用例:");
  puts("  $ group03.exe enshu3.txt");
  puts("  $ group03.exe -m 120 -f enshu3.txt -o out.txt\n");

  puts("補足:");
  puts("  同じオプションを複数回指定した場合は、後の指定を優先します");
}


/*!
 * 結果をファイルに出力する
 * @param [in] f             出力ファイルにファイルポインタ
 * @param [in] feature_datas 特徴データの配列
 * @param [in] len           特徴データの要素数
 */
static void write_features(FILE *f, const feature *feature_datas, unsigned int len) {
  unsigned int i;
  fprintf(f, "%lf %lf %lf\n",
      feature_datas->time,
      feature_datas->len,
      feature_datas->area);
  for (i = 1, feature_datas++; i < len; i++, feature_datas++) {
    fprintf(f, "%lf %lf %lf %lf\n",
        feature_datas->time,
        feature_datas->len,
        feature_datas->area,
        feature_datas->cog_change);
  }
}
