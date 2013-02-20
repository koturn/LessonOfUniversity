/*!
 * @brief wcコマンドの独自実装
 * @file    mywc.c
 * @author  1G101097
 * @version 1.1
 */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


/*! @brief wc_infoの再帰代入演算マクロ */
#define REC_ASSIGN_WC_INFO(op, wi1, wi2) {  \
  (wi1)->n_byte op##= (wi2)->n_byte;        \
  (wi1)->n_line op##= (wi2)->n_line;        \
  (wi1)->n_word op##= (wi2)->n_word;        \
}

#ifndef TRUE
#define TRUE  1  /*!< @brief TRUEの値 */
#endif
#ifndef FALSE
#define FALSE 0  /*!< @brief FALSEの値 */
#endif


typedef unsigned int uint;  /*!< @brief unsigned intのショートエイリアス */

/*! @brief ファイルのバイト数、行数、単語数を格納する */
typedef struct {
  uint n_byte;  /*!< @brief ファイルのバイトを格納する */
  uint n_line;  /*!< @brief ファイルの行数を格納する */
  uint n_word;  /*!< @brief ファイルの単語数を格納する */
} wc_info;

/*! @brief オプションのビットフラグについての列挙体 */
enum opt_bitflag {
  BYTE_FLAG = 0x01,  /*!< @brief バイト数を表示するかどうかのフラグ */
  LINE_FLAG = 0x02,  /*!< @brief 行数を表示するかどうかのフラグ */
  WORD_FLAG = 0x04   /*!< @brief 単語数を表示するかどうかのフラグ */
};

/*! @brief 文字の種類についての列挙体 */
enum chartype {
  IS_EOL,          /*!< @brief 直前の文字が改行記号だったことを示す */
  IS_WHITE_SPACE,  /*!< @brief 直前の文字が空白記号だったことを示す */
  IS_CHARACTER,    /*!< @brief 直前の文字が文字だったことを示す */
  IS_DEFAULT       /*!< @brief 直前の文字が無い場合(最初の状態) */
};


static int  parse_args(int argc, char *argv[], uint *r_index);
static void analyze_file(FILE *fp, wc_info *wi);
static void print_fileinfo(const char *filename, int opt_flag, const wc_info *wi);


/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の数
 * @param [in] argv コマンドライン引数
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  uint    r_index;
  wc_info wi       = {0, 0, 0};
  wc_info total_wi = {0, 0, 0};
  int     i;
  int     opt_flag;

  opt_flag = parse_args(argc, argv, &r_index);
  if ((int)r_index == argc) {
    analyze_file(stdin, &wi);
    print_fileinfo("", opt_flag, &wi);
    return EXIT_SUCCESS;
  }

  for (i = r_index; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
      perror(argv[0]);
      continue;  /* 次のファイルを調べる */
    }
    analyze_file(fp, &wi);
    print_fileinfo(argv[i], opt_flag, &wi);
    REC_ASSIGN_WC_INFO(+, &total_wi, &wi)
    fclose(fp);
  }
  if (argc - r_index != 1) {
    print_fileinfo("total", opt_flag, &total_wi);
  }
  return EXIT_SUCCESS;
}


/*!
 * @brief コマンドライン引数を解析する
 *
 * オプションでない引数は後ろに並び替えられる。
 * それらの引数の開始インデックスをr_indexに格納する。
 * @param [in]     argc    コマンドライン引数の数
 * @param [in,out] argv    コマンドライン引数
 * @param [out]    r_index 残った引数のインデックス
 * @return 表示する項目についてのフラグ
 */
static int parse_args(int argc, char *argv[], uint *r_index) {
  int  opt_flag = 0;
  char optch;

  while ((optch = getopt(argc, argv, "clw")) != -1) {
    switch (optch) {
      case 'c':
        opt_flag |= BYTE_FLAG;
        break;
      case 'l':
        opt_flag |= LINE_FLAG;
        break;
      case 'w':
        opt_flag |= WORD_FLAG;
        break;
    }
  }
  *r_index = optind;  /* オプション以外の引数の開始位置をr_indexに代入 */
  return opt_flag == 0x00 ? (BYTE_FLAG | LINE_FLAG | WORD_FLAG) : opt_flag;
}


/*!
 * @brief ファイルを解析し、ファイルサイズ、行数、単語数を得る
 * @param [in] fp 読み取るファイルのポインタ
 * @param [in] wi ファイルサイズ、行数、単語数を格納する構造体
 */
static void analyze_file(FILE *fp, wc_info *wi) {
  char ch;
  char prev_type = IS_DEFAULT;
  wi->n_byte = 0;
  wi->n_line = 0;
  wi->n_word = 0;
  while ((ch = fgetc(fp)) != EOF) {
    wi->n_byte++;
    switch (ch) {
      case '\n':  /* 改行記号 */
        (wi->n_line)++;
        prev_type = IS_EOL;
        break;
      case ' ':   /* fall-throughで下に落とす */
      case '\t':
        prev_type = IS_WHITE_SPACE;
        break;
      default:    /* 文字の場合 */
        if (prev_type != IS_CHARACTER) (wi->n_word)++;
        prev_type = IS_CHARACTER;
        break;
    }
  }
}


/*!
 * @brief ファイルのサイズ、行数、単語数を表示する
 * @param [in] filename ファイル名
 * @param [in] opt_flag どれを表示するかを格納したフラグ変数
 * @param [in] wi       ファイルのバイト数、単語数、行数を格納した構造体
 */
static void print_fileinfo(const char *filename, int opt_flag, const wc_info *wi) {
  if (opt_flag & LINE_FLAG) printf("%u  ", wi->n_line);
  if (opt_flag & WORD_FLAG) printf("%u  ", wi->n_word);
  if (opt_flag & BYTE_FLAG) printf("%u  ", wi->n_byte);
  puts(filename);
}
