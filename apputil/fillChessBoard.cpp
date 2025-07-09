#include "fillChessBoard.h"
#include "parallelWithBarrier.h"

void fillChessBoard(float *img, int w, int h, int stride, int boxSize){
    std::atomic<size_t> row{0};
    auto w3 = w*3;
    auto boxSize3 = boxSize*3;
    parallel([&](size_t i){
        for( auto r = row++; r<h; r = row++){
            auto rp = img + r*stride*3;
            for(int c=0; c < w3; c+=3){
                auto fill = (c/boxSize3)%2 == (r/boxSize)%2;
                rp[c+0] = (fill? 1:0);
                rp[c+1] = rp[c+2] = (fill? 0:1);
            }
        }
    });
}
