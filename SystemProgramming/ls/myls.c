/*!
 * @brief lsコマンドの独自実装
 * @file myls.c
 * @version 1.1
 * @author  1G101097
 */
#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define ARRAY_SIZE  1024  /*!< @brief 配列の要素数 */
#define NUM_LENGTH  10    /*!< @brief ユーザID */
#define MODE_LENGTH 11    /*!< @brief 配列の要素数 */
#define MALLOC(type, n)  ((type *)malloc(sizeof(type) * (n)))  /*!< @brief malloc()関数のラッパーマクロ */

typedef unsigned int uint;  /*!< @brief unsigned intのショートエイリアス */

/*! @brief オプションのビットフラグについての列挙体 */
enum opt_bitflag {
  ALL_FLAG      = 0x01,  /*!< @brief -aオプションに関するビット */
  INFO_FLAG     = 0x02,  /*!< @brief -lオプションに関するビット */
  RECCURSE_FLAG = 0x04   /*!< @brief -Rオプションに関するビット */
};


static int   parse_args(int argc, char *argv[], uint *r_index);
static void  trim_dirname(char *dirname);
static void  do_ls(const char dirname[], const char relpath[], int opt_flag);
static int   cmpfunc(const void *direntp1, const void *direntp2);
static void  show_list(struct dirent *const dirent_array[], const char relapth[], uint end_idx, int opt_flag);
static void  show_file_info(const char *filename, const struct stat *info_p);
static void  mode_to_letters(int mode, char str[]);
static char *uid_to_name(uid_t uid);
static char *gid_to_name(gid_t gid);
static void  make_relpath(char *new_relpath, const char *relpath, const char *dirname);




/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の数
 * @param [in] argv コマンドライン引数
 * @return 終了コード
 */
int main(int argc, char *argv[]) {
  static char curdir_name[ARRAY_SIZE];
  uint r_index;
  int  opt_flag = parse_args(argc, argv, &r_index);

  switch (argc - r_index) {
    case 0:
      if (opt_flag & RECCURSE_FLAG) {
        puts(".:");
      }
      do_ls(".", ".", opt_flag);
      break;
    case 1:
      trim_dirname(argv[r_index]);
      if (opt_flag & RECCURSE_FLAG) {
        printf("%s:\n", argv[r_index]);
      }
      getcwd(curdir_name, sizeof(curdir_name));  /* カレントディレクトリを保存 */
      chdir(argv[r_index]);                      /* 対象ディレクトリに移動 */
      do_ls(".", argv[r_index], opt_flag);       /* 移動先のディレクトリをカレントディレクトリとしてls */
      chdir(curdir_name);                        /* 元のディレクトリに戻る */
      break;
    default:
      for (; r_index < (uint)argc; r_index++) {
        trim_dirname(argv[r_index]);
        printf("%s:\n", argv[r_index]);            /* 対象ディレクトリが2つ以上なので、どのディレクトリが表示 */
        getcwd(curdir_name, sizeof(curdir_name));  /* カレントディレクトリを保存 */
        chdir(argv[r_index]);                      /* 対象ディレクトリに移動 */
        do_ls(".", argv[r_index], opt_flag);       /* 移動先のディレクトリをカレントディレクトリとしてls */
        chdir(curdir_name);                        /* 元のディレクトリに戻る */
      }
      break;
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
  while ((optch = getopt(argc, argv, "alR")) != -1) {
    switch (optch) {
      case 'a':
        opt_flag |= ALL_FLAG;
        break;
      case 'l':
        opt_flag |= INFO_FLAG;
        break;
      case 'R':
        opt_flag |= RECCURSE_FLAG;
        break;
    }
  }
  *r_index = optind;  /* オプション以外の引数の開始位置をr_indexに代入 */
  return opt_flag;
}


/*!
 * @brief ディレクトリ名末尾に'/'があれば、取り除く
 * @param [out] dirname ディレクトリ名
 */
static void trim_dirname(char *dirname) {
  uint idx = strlen(dirname) - 1;
  if (dirname[idx] == '/') {
    dirname[idx] = '\0';
  }
}


/*!
 * @brief 指定ディレクトリ内のファイルのリストを表示する
 * @param [in] dirname  ディレクトリ名
 * @param [in] relpath  相対パス名
 * @param [in] opt_flag オプションのビットフラグ
 */
static void do_ls(const char dirname[], const char relpath[], int opt_flag) {
  DIR           *dir_ptr;                   /* ディレクトリのポインタ */
  struct dirent *dirent_array[ARRAY_SIZE];  /* ディレクトリ内のエントリ情報の配列 */

  if ((dir_ptr = opendir(dirname)) == NULL) {
    perror("opendir");
  } else {
    uint end_idx = 0;
    for (end_idx = 0; (dirent_array[end_idx] = readdir(dir_ptr)) != NULL; end_idx++);

    qsort(dirent_array, end_idx, sizeof(struct dirent *), cmpfunc);
    show_list(dirent_array, relpath, end_idx, opt_flag);
    closedir(dir_ptr);
  }
}


/*!
 * @brief (struct dirent *)のqsort用比較関数
 *
 * dirent構造体のd_nameで比較する
 * @param [in] direntp1 比較対象1
 * @param [in] direntp2 比較対象2
 * @return 比較値(strcmp返り値)
 */
static int cmpfunc(const void *direntp1, const void *direntp2) {
  return strcmp((*(struct dirent *const *)direntp1)->d_name,
      (*(struct dirent *const *)direntp2)->d_name);
}


/*!
 * @brief ファイルのリストを表示する
 * @param [in] dirent_array ディレクトリ中のファイル一覧
 * @param [in] relpath      相対パス名
 * @param [in] end_idx      ディレクトリ中のファイルの個数
 * @param [in] opt_flag     オプションによって指定されるビットフラグ
 */
static void show_list(struct dirent *const dirent_array[], const char relpath[], uint end_idx, int opt_flag) {
  uint i;
  char        *filename;
  struct stat *infos = MALLOC(struct stat, end_idx);

  if (infos == NULL) {
    fputs("メモリ確保エラーです", stderr);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < end_idx; i++) {
    char *filename = dirent_array[i]->d_name;
    if (stat(filename, &infos[i]) == -1) {  /* stat()に失敗した場合 */
      perror(filename);
      return;
    }
  }

  if (opt_flag & INFO_FLAG) {  /* -lオプションが指定されている場合 */
    for (i = 0; i < end_idx; i++) {
      filename = dirent_array[i]->d_name;
      if (filename[0] == '.' && !(opt_flag & ALL_FLAG)) continue;
      show_file_info(filename, &infos[i]);
    }
  } else {
    for (i = 0; i < end_idx; i++) {
      filename = dirent_array[i]->d_name;
      if (filename[0] == '.' && !(opt_flag & ALL_FLAG)) continue;
      puts(filename);
    }
  }

  /* -Rオプションが指定されている場合 */
  if (opt_flag & RECCURSE_FLAG) {
    for (i = 0; i < end_idx; i++) {
      filename = dirent_array[i]->d_name;
      if (S_ISDIR(infos[i].st_mode) && strcmp(filename, ".") && strcmp(filename, "..")) {
        char curdir_name[ARRAY_SIZE];
        char new_relpath[ARRAY_SIZE];
        make_relpath(new_relpath, relpath, filename);
        printf("\n%s:\n", new_relpath);            /* 対象ディレクトリが2つ以上なので、どのディレクトリが表示 */

        getcwd(curdir_name, sizeof(curdir_name));  /* カレントディレクトリを保存 */
        chdir(filename);                           /* 対象ディレクトリに移動 */
        do_ls(".", new_relpath, opt_flag);         /* 移動先のディレクトリをカレントディレクトリとしてls */
        chdir(curdir_name);                        /* 元のディレクトリに戻る */
      }
    }
  }
  free(infos);
}


/*!
 * @brief ファイル名とファイルの情報を表示する
 * @param [in] filename ファイル名
 * @param [in] info_p   ファイル情報
 */
static void show_file_info(const char *filename, const struct stat *info_p) {
  char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
  void  mode_to_letters();
  char  modestr[MODE_LENGTH];

  mode_to_letters(info_p->st_mode, modestr);

  printf("%s"    , modestr);
  printf("%4d "  , (int)info_p->st_nlink);
  printf("%-8s " , uid_to_name(info_p->st_uid));
  printf("%-8s " , gid_to_name(info_p->st_gid));
  printf("%8ld " , (long)info_p->st_size);
  printf("%.12s ", 4 + ctime(&info_p->st_mtime));
  printf("%s\n"  , filename);
}


/*!
 * @brief ファイルのモード(パーミッションなど)の文字列を得る
 * @param [in]  mode モードを表すビットフラグ整数
 * @param [out] str  モード情報を格納する文字配列
 */
static void mode_to_letters(int mode, char str[]) {
  strcpy(str, "----------");           /* default=no perms */

  if (S_ISDIR(mode))  str[0] = 'd';    /* ディレクトリかどうか */
  if (S_ISCHR(mode))  str[0] = 'c';    /* キャラクタデバイスかどうか */
  if (S_ISBLK(mode))  str[0] = 'b';    /* ブロックデバイスかどうか */

  /* ----- ユーザのパーミッション ----- */
  if (mode & S_IRUSR) str[1] = 'r';
  if (mode & S_IWUSR) str[2] = 'w';
  if (mode & S_IXUSR) str[3] = 'x';

  /* ----- ユーザグループのパーミッション ----- */
  if (mode & S_IRGRP) str[4] = 'r';
  if (mode & S_IWGRP) str[5] = 'w';
  if (mode & S_IXGRP) str[6] = 'x';

  /* ----- 他グループのパーミッション ----- */
  if (mode & S_IROTH) str[7] = 'r';
  if (mode & S_IWOTH) str[8] = 'w';
  if (mode & S_IXOTH) str[9] = 'x';
}


/*!
 * @brief ユーザ名を得る
 * @param [in] uid ユーザID
 * @return ユーザ名の文字列へのポインタ
 */
static char *uid_to_name(uid_t uid) {
  struct passwd *getpwuid(), *pw_ptr;
  static char numstr[NUM_LENGTH];

  if ((pw_ptr = getpwuid(uid)) == NULL) {
    sprintf(numstr, "%u", uid);
    return numstr;
  } else {
    return pw_ptr->pw_name;
  }
}


/*!
 * @brief グループナンバーを得る。
 * @param [in] gid グループID
 * @return グループ名
 */
static char *gid_to_name(gid_t gid) {
  struct group *getgrgid(), *grp_ptr;
  static  char numstr[NUM_LENGTH];

  if ((grp_ptr = getgrgid(gid)) == NULL) {
    sprintf(numstr, "%u", gid);
    return numstr;
  } else {
    return grp_ptr->gr_name;
  }
}


/*!
 * @brief 新しい相対パスを作成する
 * @param [out] new_relpath 新しい相対パスを格納する配列
 * @param [in]  relpath     今までの相対パス
 * @param [in]  dirname     末尾に付け加えるディレクトリ名
 */
static void make_relpath(char *new_relpath, const char *relpath, const char *dirname) {
  strcpy(new_relpath, relpath);
  strcat(new_relpath, "/");
  strcat(new_relpath, dirname);
}
