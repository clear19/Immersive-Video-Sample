//
// Created by Administrator on 2020/7/8.
//

#ifndef IMMERSIVE_VIDEO_SAMPLE_HEVCFRAMESPLITER_H
#define IMMERSIVE_VIDEO_SAMPLE_HEVCFRAMESPLITER_H
#include <stdio.h>
#include <stdint.h>

typedef struct HEVC_BUF
{
    uint8_t  *buf;
    uint64_t  size;
}HevcBuf;

class HevcFrameSplitter {
public:
    HevcFrameSplitter(const char * fileName);
    ~HevcFrameSplitter();

    int getOneFrame(HevcBuf* hevcBuf);

private:
    HevcBuf           mInputBuffer;
    HevcBuf           mOutputBuffer;
    HevcBuf           mParameterSets;
    HevcBuf           mNalBuffer;
    FILE*              mInputFile;
};


#endif //IMMERSIVE_VIDEO_SAMPLE_HEVCFRAMESPLITER_H
