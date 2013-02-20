#include <math.h>
#include <stdio.h>
#include "data_handler.h"

#define BUF_SIZE  512
#define DATA_COL   10
#define SQUARE(n) ((n) * (n))


#ifndef OPTIMIZE
static double calc_dist(const position *pos1, const position *pos2);
static void   calc_cog(position *cog_pos, const data_fmt *datas);

#else
// 上記の関数に代わるインラインマクロ
// 上記の関数と同じAPIにしてある
// コール時間の削減のために提供
#define calc_dist(pos1, pos2)  \
  (sqrt(SQUARE((pos2)->x - (pos1)->x) + SQUARE((pos2)->y - (pos1)->y) + SQUARE((pos2)->z - (pos1)->z)))

#define calc_cog(cog_pos, data)  {                                       \
  (cog_pos)->x = ((data)->pos1.x + (data)->pos2.x + (data)->pos3.x) / 3; \
  (cog_pos)->y = ((data)->pos1.y + (data)->pos2.y + (data)->pos3.y) / 3; \
  (cog_pos)->z = ((data)->pos1.z + (data)->pos2.z + (data)->pos3.z) / 3; \
}

#endif




/**
 * csvファイルを読み込む
 * @param [in] f     csvファイルのファイルポインタ
 * @param [in] datas csvファイルを格納する配列
 * @return 有効データ数
 */
unsigned int read_csv(FILE *f, data_fmt *datas) {
  unsigned int  cnt     = 0;    // 有効データ数のカウンタ
  unsigned int  line_no = 0;    // 入力ファイルの行番号(エラー出力に用いる)
  static   char buf[BUF_SIZE];  // 読み込み用バッファ

  while (fgets(buf, sizeof(buf), f) != NULL) {
    int match = sscanf(buf, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
        &datas->time,
        &datas->pos1.x, &datas->pos1.y, &datas->pos1.z,
        &datas->pos2.x, &datas->pos2.y, &datas->pos2.z,
        &datas->pos3.x, &datas->pos3.y, &datas->pos3.z);
    line_no++;
    if (match == DATA_COL) {  // データファイルの1行が正しいフォーマットとマッチするなら、
      cnt++;    // 有効データ数をインクリメント
      datas++;  // 読み込み用のデータアドレスを次に進める
    } else {
      fprintf(stderr, "Invalid format data at line %d ... ignored!\n", line_no);
    }
  }
  return cnt;  // 有効データ数を返す
}


/*!
 * ダウンサンプリングを行う。
 * @param [out] down_smpl_datas ダウンサンプリングデータを格納する配列
 * @param [in]  datas           オリジナルのデータ
 * @param [in]  len             オリジナルのデータ数
 * @param [in]  merge_num       結合する数
 */
void down_sample(data_fmt *down_smpl_datas, const data_fmt *datas, unsigned int len, unsigned int merge_num) {
  static const data_fmt ZERO_DATA = {0.0, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  unsigned int i;
  unsigned int repeat = len / merge_num;
  unsigned int rest   = len % merge_num;

  for (i = 0; i < repeat; i++, down_smpl_datas++) {
    unsigned int j;
    *down_smpl_datas = ZERO_DATA;  // down_smpl_dataの各要素にゼロをセット
    down_smpl_datas->time = datas->time;
    for (j = 0; j < merge_num; j++, datas++) {
      REC_ASSIGN_DATA2DATA(+, down_smpl_datas, datas);   // 時間を除く各要素の再帰代入演算
    }
    REC_ASSIGN_DATA2NUM(/, down_smpl_datas, merge_num);  // 時間を除く各要素の再帰代入演算
  }

  if (rest == 0) return;

  // 元のデータ数がダウンサンプリングでまとめる数で割り切れないとき、
  *down_smpl_datas = ZERO_DATA;  // down_smpl_dataの各要素にゼロをセット
  down_smpl_datas->time = datas->time;
  for (i = 0; i < rest; i++, datas++) {
    REC_ASSIGN_DATA2DATA(+, down_smpl_datas, datas);  // 時間を除く各要素の再帰代入演算
  }
  REC_ASSIGN_DATA2NUM(/, down_smpl_datas, rest);      // 時間を除く各要素の再帰代入演算
}


/*!
 * 特徴データを引き出す
 * @param [out] feature_datas 特徴データを格納する配列
 * @param [in]  datas         特徴データを抜き出す元となるデータ
 * @param [in]  len           データ数
 */
void derive_features(feature *feature_datas, const data_fmt *datas, unsigned int len) {
  unsigned int i;
  position prev_cog_pos = {0.0, 0.0, 0.0};  // 1つ前のステップの重心位置記憶用変数

  feature_datas->cog_change = 0.0;  // 最初の重心位置変化は0.0とする
  for (i = 0; i < len; i++, feature_datas++, datas++) {
    position cog_pos;
    double   s;
    double   dist1 = calc_dist(&datas->pos1, &datas->pos2);
    double   dist2 = calc_dist(&datas->pos2, &datas->pos3);
    double   dist3 = calc_dist(&datas->pos3, &datas->pos1);

    feature_datas->time = datas->time;            // 時間の代入
    feature_datas->len  = dist1 + dist2 + dist3;  // 距離の総和の計算

    // ヘロンの公式により、三角形の面積を計算
    s = feature_datas->len / 2;
    feature_datas->area = sqrt(s * (s - dist1) * (s - dist2) * (s - dist3));

    // 重心位置の変化を計算
    calc_cog(&cog_pos, datas);
    if (i > 0) {
      // 重心位置の変化量(距離を記憶)
      feature_datas->cog_change = calc_dist(&cog_pos, &prev_cog_pos);
    }
    prev_cog_pos = cog_pos;  // 現在の重心位置を記憶(次のステップで用いる)
  }
}




#ifndef OPTIMIZE
/*!
 * 2点間の距離を計算する
 * @deprecated インラインマクロが用意されています
 * @param [in] pos1 座標点1
 * @param [in] pos2 座標点2
 * @return pos1とpos2の距離
 */
static double calc_dist(const position *pos1, const position *pos2) {
  return sqrt(SQUARE(pos2->x - pos1->x) + SQUARE(pos2->y - pos1->y) + SQUARE(pos2->z - pos1->z));
}


/*!
 * 重心(Center Of Gravity)を計算する
 * @deprecated インラインマクロが用意されています
 * @param [out] cog_pos 重心座標を格納するposition構造体
 * @param [in]  data    重心を計算したい(1つの)データ
 */
static void calc_cog(position *cog_pos, const data_fmt *data) {
  cog_pos->x = (data->pos1.x + data->pos2.x + data->pos3.x) / 3;
  cog_pos->y = (data->pos1.y + data->pos2.y + data->pos3.y) / 3;
  cog_pos->z = (data->pos1.z + data->pos2.z + data->pos3.z) / 3;
}
#endif
