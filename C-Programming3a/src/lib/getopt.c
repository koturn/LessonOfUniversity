#include <stdio.h>
#include <string.h>
#include "getopt.h"

#define ERR(s, c) { if (opterr) fprintf (stderr, "%s%s%c\n", argv[0], s, c); }

static int  opterr = 1;
static int  optind = 1;
static int  optopt;
char       *optarg;


/**
 * オプション解析関数
 * gccのライブラリ<getopt.h>のgetopt()関数の独自実装
 * @param [in] argc コマンドライン引数の個数(プログラム名も含む)
 * @param [in] argv コマンドライン引数へのダブルポインタ
 * @param [in] opts オプション文字列
 * @return 解析引数がオプション文字列に含まれるならオプション文字を、それ以外なら'?'を返す
 */
int getopt(int argc, char *const *argv, const char *opts) {
  int   sp = 1;
  int   c;
  char *cp;

  if (sp == 1) {
    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
      return EOF;
    } else if(strcmp(argv[optind], "--") == 0) {
      optind++;
      return EOF;
    }
  }

  optopt = c = argv[optind][sp];
  if (c == ':' || (cp = strchr(opts, c)) == NULL) {
    ERR(": illegal option -- ", c);
    if(argv[optind][++sp] == '\0') {
      optind++;
      sp = 1;
    }
    return '?';
  }

  if (*++cp == ':') {
    if (argv[optind][sp + 1] != '\0') {
      optarg = &argv[optind++][sp + 1];
    } else if(++optind >= argc) {
      ERR(": option requires an argument -- ", c);
      sp = 1;
      return '?';
    } else {
      optarg = argv[optind++];
    }
    sp = 1;
  } else {
    if (argv[optind][++sp] == '\0') {
      sp = 1;
      optind++;
    }
    optarg = NULL;
  }
  return c;
}
