/********************************
Author: Sravanthi Kota Venkata
********************************/

#ifndef _TEXTURE_
#define _TEXTURE_

#include "sdvbs_common.h"

#define R 0
#define G 1
#define B 2
#define a(x,y,W)        (1*((y)*(W)+(x)))
#define aa(x,y) (2*((y)*data->widthout+(x)))

typedef float pixelvalue;

typedef struct
{
 double sign, diff;
 int x,y;
 int secondx, secondy;
}signature;

typedef struct{
 int localx, localy, localz;
 int widthin, widthout;
 int heightin, heightout;
 int nfin, nfout;
}params;


void create_texture(F2D *image, params *data,F2D* result,F2D* target,int* atlas,int* xloopout,int* yloopout);
I2D* parse_flags(int argc, char ** argv);
void init_params(params *data, int WIDTHin, int HEIGHTin, int WIDTH, int HEIGHT, int localx,int localy);
void init(F2D *image, params* data,int* atlas,F2D* target,F2D* result, int* vrstartx,int* vrstarty,int* vrfinishx,int* vrfinishy,int anotherpass);
double compare_full_neighb(F2D *image,int x, int y, F2D *image1,int x1, int y1, params* data,int* xloopout,int* yloopout);
double compare_neighb(F2D *image,int x, int y, F2D *image1,int x1, int y1, params* data,int* xloopout,int* yloopout);
double compare_rest(F2D *image,int x, int y, F2D *tar,int x1, int y1, params* data,int* xloopout,int* yloopout);
int create_candidates(int x,int y, params* data,int* atlas,int* xloopout,int* yloopout, int vrstartx,int vrstarty,int vrfinishx,int vrfinishy,int* candlistx, int* candlisty);
int create_all_candidates(int x,int y, params* data,int* atlas,int* xloopout,int* yloopout, int vrstartx,int vrstarty,int vrfinishx,int vrfinishy,int* candlistx, int* candlisty);

#endif



