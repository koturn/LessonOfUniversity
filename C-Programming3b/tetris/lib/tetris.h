/*!
 * @brief テトリス関係のAPIを提供するヘッダファイル
 *
 * 実態はtetirs.cにある。
 * @author 2012-C3-group03
 * @file tetris.h
 * @version 1.0
 */

#ifndef TETRIS_H
#define TETRIS_H


// ゲーム起動直後の初期設定を行う関数。画面と壁のデータを初期化
int play_tetris(int sock);

#define DEFAULT_PORT  50000  //!< デフォルトのポート番号

#endif
