/*!
 * @brief テトリスのゲームに関するソースファイル
 *
 * テトリス関係のAPIを提供している。
 * @author 2012-C3-group03
 * @file tetris.c
 * @version 1.0
 */
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "macro.h"
#include "tetris.h"


#define STAGE_WIDTH     12  //!< ステージの横幅
#define STAGE_HEIGHT    21  //!< ステージの縦幅

#define BLOCK_WIDTH      4  //!< ブロックの横幅
#define BLOCK_HEIGHT     4  //!< ブロックの縦幅
#define N_BLOCK          7  //!< ブロックの種類

#define SPACE            0  //!< ブロックが無いことを示す値
#define WALL             9  //!< 壁であることを示す値

#define MY_FIELD_X       0  //!< 自分のフィールド位置のx座標
#define MY_FIELD_Y       2  //!< 自分のフィールド位置のy座標
#define MY_SCORE_X      16  //!< 自分のスコア描画位置のx座標
#define MY_SCORE_Y      10  //!< 自分のスコア描画位置のy座標

#define ENEMY_FIELD_X   40  //!< 敵フィールドの描画位置
#define ENEMY_FIELD_Y    2  //!< 敵フィールドの描画位置
#define ENEMY_SCORE_X   16  //!< 敵フィールドの描画位置
#define ENEMY_SCORE_Y   15  //!< 敵フィールドの描画位置
#define SCORE_LEN       10  //!< スコアの文字列を収めるバッファの容量

#define CURSOR_X  (ENEMY_FIELD_X + STAGE_WIDTH)   //!< 見栄えをよくするためのカーソルのx座標
#define CURSOR_Y  (ENEMY_FIELD_Y + STAGE_HEIGHT)  //!< 見栄えをよくするためのカーソルのy座標

#define RESULT_STR_X     5  //!< 結果の表示位置のx座標
#define RESULT_STR_Y     8  //!< 結果の表示位置のy座標

#define GAME_TIME      60  //!< ゲームの制限時間(秒)
#define TIME_X          16  //!< 時間描画位置のx座標
#define TIME_Y           5  //!< 時間描画位置のx座標
#define TIME_LEN         5  //!< 時間の文字列を収めるバッファの容量


//! キーコード
enum keycode {
  CTRL_B = 0x02,  //!< Ctrl-Bを押したときに送信される値
  CTRL_F = 0x06,  //!< Ctrl-Fを押したときに送信される値
  CTRL_N = 0x0e   //!< Ctrl-Nを押したときに送信される値
};


//! 特別な送信データ
enum protocol {
  GAMEOVER  = 0xff,  //!< ゲームオーバーを表す
  INTERRUPT = 0xfe,  //!< Ctrl-Cなどで強制終了されたことを表す
  TIMEUP    = 0xfd   //!< 時間切れになったことを表す
};

//! ブロックの回転方向
typedef enum {
  RIGHT,  //!< 右回りを表す
  LEFT,   //!< 左回りを表す
} turn_drct_e;


static uchar stage[STAGE_HEIGHT][STAGE_WIDTH];  // 壁と固定済みブロック用
static uchar block[BLOCK_HEIGHT][BLOCK_WIDTH];  // 現在落下中のブロックを入れる
static uchar field[STAGE_HEIGHT][STAGE_WIDTH];  // 描画するデータ。stage[][]に block[][]を重ねたもの

//! 7種類のブロックのデータ
static uchar block_list[N_BLOCK][BLOCK_HEIGHT][BLOCK_WIDTH] = {
  {{0,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0}},
  {{0,0,0,0},
    {0,1,1,0},
    {0,1,0,0},
    {0,1,0,0}},
  {{0,0,1,0},
    {0,1,1,0},
    {0,1,0,0},
    {0,0,0,0}},
  {{0,1,0,0},
    {0,1,1,0},
    {0,0,1,0},
    {0,0,0,0}},
  {{0,0,0,0},
    {0,1,0,0},
    {1,1,1,0},
    {0,0,0,0}},
  {{0,0,0,0},
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0}},
  {{0,0,0,0},
    {0,1,1,0},
    {0,0,1,0},
    {0,0,1,0}}
};


static uint  y = 0;            //!< ブロックの画面上でのy座標
static uint  x = 4;            //!< ブロックの画面上でのx座標
static uint  score = 0;        //!< 自分のスコア
static uint  enemy_score = 0;  //!< 相手のスコア */
static int   sig_sock;         //!< シグナルハンドラが利用するソケットのディスクリプタ
static uchar gameover = FALSE; //!< ゲームオーバー判定。新しいブロックが初期位置に置けなければ1 になる。

static void  initialize(int sock);  // ゲームを初期化する
static void  print_labels(void);    // ラベルを描画する
static void  timeup(int sock);      // タイムアップ時の処理を行う
static void  control_block(void);   // キー入力に応じてブロックに移動や回転等の処理を行わせる
static void  drop_block(void);      // ブロックを落下させる。下に移動できない場合ブロックをその位置に固定
static void  show_field(uchar field[STAGE_HEIGHT][STAGE_WIDTH], uint x);  //field[][]の中身に応じて、画面を描画する
static void  print_score(uint x, uint y, uint score);  // スコアを表示する
static void  print_time(time_t time);             // 残りゲーム時間を表示する
static void  create_block(void);                  // 新しいブロックを生成して次のブロックに発生させる
static uchar check_overlap(uint x, uint y);       // 落下中のブロックが壁や固定済みブロックに接触していないか判別
static void  move_block(uint new_x, uint new_y);  // 落下中ブロックを一旦消して、任意の座標に移動させる
static uchar turn_block(turn_drct_e direction);   // ブロックの回転を処理する
static void  lock_block(void);                    // 着地したブロックを固定済みブロックに加える関数
static void  check_lines(void);                   // ブロックが横一列にそろえばそこを消去後、上のブロックをそこに下ろす
static void  sig_handler(int signum);             // シグナルハンドラ
static uchar read_protocol(uchar pnum);           // 特殊な値を受信したかチェックし、処理を行う
static ulong gettimeofday_sec(void);              // 現在の時間をミリ秒単位で得る




/*!
 * @brief ゲームのメインループ
 * @param [in] sock  送信先ソケットのディスクリプタ
 */
int play_tetris(int sock) {
  static const uchar FLAG = GAMEOVER;  /* ゲームオーバ時に送信する値 */
  uint   cnt = 1;                 /* カウンタ */
  time_t base_time = time(NULL);  /* ゲーム開始時刻を記憶 */

  initialize(sock); /* 初期化 */
  while (!gameover) {  /* ゲームオーバーになるまでゲーム続行 */
    static uchar enemy_field[STAGE_HEIGHT][STAGE_WIDTH];  /* 相手のフィールドデータを格納する2次元配列 */
    time_t game_time;        /* ゲームを開始してから、何秒経過したかを保持する */

    /* キー入力があればそれに応じて操作 */
    control_block();
    /* 32回ループをしたら、ブロックを1マス落とす */
    if ((cnt = (cnt + 1) % 32) == 0) {
      drop_block();
    }

    write(sock,  field, sizeof(field));             /* 自分のフィールドデータを送信 */
    write(sock, &score, sizeof(score));             /* 自分のスコアを送信 */
    read(sock,  enemy_field, sizeof(enemy_field));  /* 相手のフィールドデータを受信 */
    read(sock, &enemy_score, sizeof(enemy_score));  /* 相手のスコアを受信 */
    show_field(enemy_field, ENEMY_FIELD_X);  /* 相手のフィールドを描画する */

    if (read_protocol(enemy_field[0][0])) {  /* 特別な値を受信していないかチェックする */
      sleep(1);
      return EXIT_SUCCESS;  /* ゲーム終了 */
    }

    print_score(ENEMY_SCORE_X, ENEMY_SCORE_Y, enemy_score);  /* 相手のスコアを描画 */

    if ((game_time = GAME_TIME - (time(NULL) - base_time)) == 0) {
      timeup(sock);
      sleep(1);
      return EXIT_SUCCESS;  /* ゲーム終了 */
    }
    print_time(game_time);  /* ゲーム時間を表示する */

    usleep(20000);                   /* 20000マイクロ秒停止する(この間、CPUに負荷をかけない) */
  }
  write(sock, &FLAG, sizeof(FLAG));  /* 自分がゲームオーバになったことを相手に知らせる */

  clear();
  mvprintw(RESULT_STR_Y, RESULT_STR_X, "You lose!");
  refresh();
  sleep(2);
  endwin();  /* curses終了 */

  puts("\nYou lose!");
  sleep(1);
  return EXIT_SUCCESS;
}


/*!
 * @brief 様々な初期化設定を行う
 *
 * シグナルハンドラを設定し、画面を初期化する。<br>
 * また、ブロックを生成する。
 */
static void initialize(int sock) {
  uint i;

  sig_sock = sock;                   /* シグナルハンドラが参照できるように、グローバル変数にソケットを記憶 */
  signal(SIGINT, sig_handler);       /* シグナルハンドラの設定 */
  srand(gettimeofday_sec());         /* 乱数の種の設定 */
  initscr();                         /* 画面を初期化する */
  clear();
  cbreak();                          /* 入力をバッファに溜め込まないようにする */
  noecho();                          /* エコーバックを行わないようにする */
  fcntl(sock, F_SETFL, O_NONBLOCK);  /* sockからのread()をノンブロッキングに(リアルタイム処理を可能にする) */
  nodelay(stdscr, TRUE);             /* getch()をノンブロッキングに */
  print_labels();                    /* ラベルを描画する */

  /* 画面と壁を初期設定 */
  for (i = 0; i < STAGE_HEIGHT; i++) {
    uint j;
    for (j = 0; j < STAGE_WIDTH; j++) {
      if ((j == 0) || (j == STAGE_WIDTH - 1) || (i == STAGE_HEIGHT - 1)) {
        field[i][j] = stage[i][j] = WALL;
      } else {
        field[i][j] = stage[i][j] = SPACE;
      }
    }
  }
  create_block();                 /* 最初のブロック発生させる */
  show_field(field, MY_FIELD_X);  /* ゲーム直後の画面を描画 */
}


/*!
 * @brief ラベルを描画する
 * @see initialize()
 */
static void print_labels(void) {
  mvprintw(0, MY_FIELD_X, "your field");
  mvprintw(0, ENEMY_FIELD_X, "enemy_field");
  mvprintw(TIME_Y - 1, TIME_X - 2, "time:");
  mvprintw(MY_SCORE_Y - 1, MY_SCORE_X - 2, "my score:");
  mvprintw(ENEMY_SCORE_Y - 1, ENEMY_SCORE_X - 2, "enemy's score:");
  print_score(MY_SCORE_X, MY_SCORE_Y, 0);
  print_score(ENEMY_SCORE_X, ENEMY_SCORE_Y, 0);
  mvprintw(ENEMY_SCORE_Y + 2, ENEMY_SCORE_X - 2, "h : move left");
  mvprintw(ENEMY_SCORE_Y + 3, ENEMY_SCORE_X - 2, "l : move right");
  mvprintw(ENEMY_SCORE_Y + 4, ENEMY_SCORE_X - 2, "j : drop a block");
  mvprintw(ENEMY_SCORE_Y + 5, ENEMY_SCORE_X - 2, "a : right-handed rotation");
  mvprintw(ENEMY_SCORE_Y + 6, ENEMY_SCORE_X - 2, "s : left-handed  rotation");
}


/*!
 * @brief タイムアップ時の処理を行う
 * @param [in] sock ソケットのディスクリプタ(クローズ用)
 */
static void timeup(int sock) {
  static const uchar FLAG = TIMEUP;
  char *msg;

  write(sock, &FLAG, sizeof(FLAG));  /* タイムアップになったことを相手に知らせる */
  sleep(1);

  clear();
  if (score > enemy_score) {
    msg = "You win!";
  } else if (score < enemy_score) {
    msg = "You lose!";
  } else {
    msg = "Draw Game!";
  }
  mvprintw(RESULT_STR_Y, RESULT_STR_X, msg);
  refresh();
  sleep(2);
  endwin();  /* curses終了 */

  puts(msg);
}


/*!
 * @brief キー入力に応じてブロックを処理
 *
 * キー操作により、ブロックの操作を行う
 * ブロックの移動方法はviエディタに似せた
 */
static void control_block(void) {
  switch (getch()) {
    /* ----- vi風 ----- */
    case 'l':
      if (!check_overlap(x + 1, y)) {
        move_block(x + 1,  y);
      }
      break;
    case 'h':
      if (!check_overlap(x - 1, y)) {
        move_block(x - 1,  y);
      }
      break;
    case 'j':
      if (!check_overlap(x, y + 1)) {
        move_block(x,  y + 1);
      }
      break;
    /* ----- Emacs風 ----- */
    case CTRL_F:
      if (!check_overlap(x + 1, y)) {
        move_block(x + 1,  y);
      }
      break;
    case CTRL_B:
      if (!check_overlap(x - 1, y)) {
        move_block(x - 1,  y);
      }
      break;
    case CTRL_N:
      if (!check_overlap(x, y + 1)) {
        move_block(x,  y + 1);
      }
      break;
    /* ----- ブロックの回転 ----- */
    case 'a':
      turn_block(RIGHT);
      break;
    case 's':
      turn_block(LEFT);
      break;
    case ' ':
      turn_block(RIGHT);
      break;
  }
}


/*!
 * @brief ブロックを落下させる
 *
 * ブロックの重なりも検出する。
 * @see check_overlap()
 */
static void drop_block(void) {
  if (!check_overlap(x, y + 1)) {  /* 重なりがなければ移動 */
    move_block(x, y + 1);
  } else {                         /* 重なりがあれば壁にする */
    lock_block();
    create_block();
    show_field(field, MY_FIELD_X);
  }
}


/*!
 * @brief スコアを表示する
 * @param [in] x     描画位置x座標
 * @param [in] y     描画位置y座標
 * @param [in] score スコア
 */
static void print_score(uint x, uint y, uint score) {
  static char score_str[SCORE_LEN];
  sprintf(score_str, "%5u", score);  /* scoreを文字列にして、score_strに格納 */
  mvprintw(y, x, score_str);
}


/*!
 * @brief ゲームの残り時間を表示する
 * @param [in] time ゲームの残り時間
 */
static void print_time(time_t time) {
  static char time_str[TIME_LEN];
  sprintf(time_str, "%3ld", time);  /* timeを文字列にして、time_strに格納 */
  mvprintw(TIME_Y, TIME_X, time_str);
}


/*!
 * @brief ブロックを生成する
 *
 * ゲームオーバー判定も行う
 */
static void create_block(void) {
  uint i;
  int block_type;  /*ブロックの種類用。0 ~ 6の乱数を入れる */
  /* まずブロックの座標を初期位置にリセット */
  y = 0;
  x = 4;

  /* 乱数を発生させ、その乱数を7で割った余り(0 ~ 6まで)でブロックの種類を決定 */
  block_type = rand() % N_BLOCK;
  /* ブロックデータの中から block_type に応じた種類のブロックを読み込む */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      block[i][j] = block_list[block_type][i][j];
    }
  }
  /* 壁+ブロックをフィールドへ */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      field[i][j + 4] = stage[i][j + 4] + block[i][j];
      /* 初期位置に置いたブロックが既に固定ブロックに重なっていればゲームオーバー */
      if (field[i][j + 4] > 1) {
        gameover = TRUE;
        return;
      }
    }
  }
  srand(gettimeofday_sec());
}


/*!
 * @brief フィールドを描画する
 * @param [in] field 描画するフィールド
 * @param [in] x     どの位置に描画するか
 */
void show_field(uchar field[STAGE_HEIGHT][STAGE_WIDTH], uint x) {
  uint i;
  for (i = 0; i < STAGE_HEIGHT; i++) {
    uint j;
    move(i + MY_FIELD_Y, x);
    for (j = 0; j < STAGE_WIDTH; j++) {
      switch (field[i][j]) {
        case SPACE:
          addch(' ');
          break;
        case WALL:
          addch('x');
          break;
        default:
          addch('o');
          break;
      }
    }
  }
  //得点表示
  print_score(MY_SCORE_X, MY_SCORE_Y, score);

  move(CURSOR_Y, CURSOR_X);
  refresh();
  //ゲームオーバーの場合は GAME OVER を表示
  /*
  if (gameover) {
    clear();
    mvprintw(RESULT_STR_Y, RESULT_STR_X, "You lose!");
    // printf("\n\n\n\n\nGAME OVER\n\n");
  }
  */
}


/*!
 * @brief ブロックが重なっているかどうかを確認する
 * @param [in] x 基準位置のx座標
 * @param [in] y 基準位置のy座標
 * @return 重なっているならTRUEを、重なっていないならFALSEを返す
 */
static uchar check_overlap(uint x, uint y) {
  uint i;
  /* ブロックが向かう位置に、固定ブロックもしくは壁があるかどうかを検査 */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      if (block[i][j]) {
        if (stage[y + i][x + j] != 0) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}


/*!
 * @brief ブロックを移動する
 * @param [in] new_x 基準位置のx座標
 * @param [in] new_y 基準位置のy座標
 */
static void move_block(uint new_x, uint new_y) {
  uint i;
  /* 今までのブロックを消す */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      field[y + i][x + j] -= block[i][j];
    }
  }

  /* ブロックの座標を更新 */
  x = new_x;
  y = new_y;

  /* 新しい座標にブロックを入れ直す */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      field[y + i][x + j] += block[i][j];
    }
  }
  show_field(field, MY_FIELD_X);
}


/*!
 * @brief ブロックを回転させる
 * @return 回転できなかったとき1を、回転できたとき0を返す。
 */
static uchar turn_block(turn_drct_e direction) {
  static uchar temp[BLOCK_HEIGHT][BLOCK_WIDTH];  /*ブロックを一時保存するための配列 */
  uint i;
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      temp[i][j] = block[i][j];
    }
  }
  /* ブロックを回転 */
  if (direction == RIGHT) {  /* 右回りに回転 */
    for (i = 0; i < BLOCK_HEIGHT; i++) {
      uint j;
      for (j = 0; j < BLOCK_WIDTH; j++) {
        block[i][j] = temp[(BLOCK_WIDTH - 1) - j][i];
      }
    }
  } else {                   /* 左回りに回転 */
    for (i = 0; i < BLOCK_HEIGHT; i++) {
      uint j;
      for (j = 0; j < BLOCK_WIDTH; j++) {
        block[i][j] = temp[j][(BLOCK_WIDTH - 1) - i];
      }
    }
  }

  /* 重なってるブロックが出てしまったらブロックを回転前に戻して中止 */
  if (check_overlap(x, y)) {
    for (i = 0; i < BLOCK_HEIGHT; i++) {
      uint j;
      for (j = 0; j < BLOCK_WIDTH; j++) {
        block[i][j] = temp[i][j];
      }
    }
    return FALSE;
  }
  /* 一旦フィールドからブロック消して回転後のブロックを再表示 */
  for (i = 0; i < BLOCK_HEIGHT; i++) {
    uint j;
    for (j = 0; j < BLOCK_WIDTH; j++) {
      field[y + i][x + j] -= temp[i][j];
      field[y + i][x + j] += block[i][j];
    }
  }
  show_field(field, MY_FIELD_X);
  return TRUE;
}


/*!
 * @brief 着地後のブロックを固定する
 *
 * また、横一列がそろってるかの判定をする。
 * @see check_lines()
 */
static void lock_block(void) {
  uint i;
  /* ブロックを壁に加える */
  for (i = 0; i < STAGE_HEIGHT; i++) {
    uint j;
    for (j = 0; j < STAGE_WIDTH; j++) {
      stage[i][j] = field[i][j];
    }
  }
  check_lines(); /* 横一列がそろってるか判定して処理する */
  /* 列完成判定後の壁をフィールドへ */
  for (i = 0; i < STAGE_HEIGHT; i++) {
    uint j;
    for (j = 0; j < STAGE_WIDTH; j++) {
      field[i][j] = stage[i][j];
    }
  }
}


/*!
 * @brief 横一列が完成しているか検査する
 *
 * 揃っていればそこを消して上のブロック群を下ろす
 */
static void check_lines(void) {
  uchar comp;      /* 横一列がそろっていれば1、一つでも隙間があると0になる */
  uint  lines = 0; /* 同時に消したラインの数 */
  while (1) {
    uint i, j;
    for (i = 0; i < STAGE_HEIGHT - 1; i++) {
      comp = TRUE;
      for (j = 1; j < STAGE_WIDTH - 1; j++) {
        if (stage[i][j] == 0) {
          comp = FALSE;
        }
      }
      if (comp == TRUE) break;
    }
    if (comp == FALSE) break;

    lines++;
    for (j = 1; j < STAGE_WIDTH - 1; j++) {  /* 列を消去 */
      stage[i][j] = 0;
    }
    /* 消えた列より上にあった固定ブロックを列の消えたところへ下ろす */
    for (j = i; j > 0; j--) {
      uint k;
      for (k = 1; k < STAGE_WIDTH - 1; k++) {
        stage[j][k] = stage[j - 1][k];
      }
    }
  }
  /* 同時に消したラインの数をカウント */
  switch (lines) {
    case 1:
      score += 100;
      break;
    case 2:
      score += 300;
      break;
    case 3:
      score += 500;
      break;
    case 4:
      score += 1000;
      break;
  }
}


/*!
 * @brief シグナルハンドラ
 *
 * 強制的に終了されたときに呼び出される
 * @param [in] signum シグナル番号
 */
static void sig_handler(int signum) {
  static const uchar FLAG = INTERRUPT;

  write(sig_sock, &FLAG, sizeof(FLAG));  /* 強制終了したことを相手に知らせる。 */
  endwin();
  sleep(1);
  close(sig_sock);
  exit(signum);
}


/*!
 * @brief 特別な値を受信したかどうかをチェックし、処理を行う
 * @param [in] pnum 受信した値の1バイト目
 */
static uchar read_protocol(uchar pnum) {
  switch (pnum) {
    case INTERRUPT:
      clear();
      mvprintw(RESULT_STR_Y, RESULT_STR_X, "Interrupted!");
      refresh();
      sleep(2);
      endwin();
      puts("対戦相手が強制終了しました");
      return TRUE;
    case GAMEOVER:
      clear();
      mvprintw(RESULT_STR_Y, RESULT_STR_X, "You win!");
      refresh();
      sleep(2);
      endwin();
      puts("\nYou win!");
      return TRUE;
    case TIMEUP:
      timeup(sig_sock);
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
