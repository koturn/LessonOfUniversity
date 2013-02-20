#pragma once


typedef struct {
  double x;
  double y;
  double z;
} position;

typedef struct {
  double   time;
  position pos1;
  position pos2;
  position pos3;
} data_fmt;

typedef struct {
  double time;
  double len;
  double area;
  double cog_change;
} feature;


// data_fmt用の再帰代入演算用マクロ(data_fmt 対 data_fmt)
#define REC_ASSIGN_DATA2DATA(op, data1, data2) { \
  (data1)->pos1.x op##= (data2)->pos1.x;         \
  (data1)->pos1.y op##= (data2)->pos1.y;         \
  (data1)->pos1.z op##= (data2)->pos1.z;         \
  (data1)->pos2.x op##= (data2)->pos2.x;         \
  (data1)->pos2.y op##= (data2)->pos2.y;         \
  (data1)->pos2.z op##= (data2)->pos2.z;         \
  (data1)->pos3.x op##= (data2)->pos3.x;         \
  (data1)->pos3.y op##= (data2)->pos3.y;         \
  (data1)->pos3.z op##= (data2)->pos3.z;         \
}

// data_fmt用の再帰代入演算用マクロ(data_fmt 対 数値)
#define REC_ASSIGN_DATA2NUM(op, data, num) { \
  (data)->pos1.x op##= (num);                \
  (data)->pos1.y op##= (num);                \
  (data)->pos1.z op##= (num);                \
  (data)->pos2.x op##= (num);                \
  (data)->pos2.y op##= (num);                \
  (data)->pos2.z op##= (num);                \
  (data)->pos3.x op##= (num);                \
  (data)->pos3.y op##= (num);                \
  (data)->pos3.z op##= (num);                \
}


unsigned int read_csv(FILE *f, data_fmt *datas);
void down_sample(data_fmt *down_smpl_datas, const data_fmt *datas, unsigned int len, unsigned int merge_num);
void derive_features(feature *feature_datas, const data_fmt *datas, unsigned int len);
