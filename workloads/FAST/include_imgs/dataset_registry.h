#ifndef DATASET_REGISTRY_H
#define DATASET_REGISTRY_H

#include "leuven_data.h"
#include "graf_data.h"
#include "bikes_data.h"

// Dataset Structure
typedef struct {
    const char* name;
    const unsigned char* data;
    int w;
    int h;
} BenchmarkImage;

static const BenchmarkImage benchmark_suite[] = {
    { "leuven", img_leuven_raw, IMG_LEUVEN_W, IMG_LEUVEN_H },
    { "graf", img_graf_raw, IMG_GRAF_W, IMG_GRAF_H },
    { "bikes", img_bikes_raw, IMG_BIKES_W, IMG_BIKES_H },
};

static const int benchmark_suite_len = 3;

#endif
