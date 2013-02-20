/*!
 * @brief 計算ゲームのヘッダファイル
 *
 * @file   calcgame.h
 * @author 2012-C3-group03
 * @version 1.0
 */

#ifndef TYPEGAME_H
#define TYPEGAME_H

#include "macro.h"

int  play_game_server(int sock_new);
int  play_game_client(int sock);
uint read_question(const char *filename, char **questions, uint len);

#define DEFAULT_PORT    50000  //!< デフォルトのポート番号

#endif
