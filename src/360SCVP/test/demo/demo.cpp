#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "../../360SCVPAPI.h"

param_360SCVP           param;
unsigned char*          pInputBuffer;
unsigned char*          pInputBufferlow;
unsigned char*          pOutputBuffer;
unsigned char*          pOutputSEI;
int                     frameWidth;
int                     frameHeight;
int                     frameWidthlow;
int                     frameHeightlow;
int                     bufferlen;
int                     bufferlenlow;
FILE*                   pInputFile;
FILE*                   pInputFileLow;
FILE*                   pOutputFile;
param_oneStream_info*   pTiledBitstreamTotal;

void init();
void deInit();
int parseOneNalu();
int parseNalu();
int processMergeAndViewport();
int processStitch();

int main() 
{
    init();

    // parseOneNalu();
    // parseNalu();
    processMergeAndViewport();
    // processStitch();

    deInit();

    return 0;
}

int parseOneNalu() 
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return ret;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    printf("pareseOneNalu nalu type=%d size=%d", nal.naluType, nal.dataSize);
    I360SCVP_unInit(pI360SCVP);
    return 0;
}

int parseNalu() 
{
    int ret = 0;
    param.paramViewPort.faceWidth = 7680;
    param.paramViewPort.faceHeight = 3840;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.paramViewPort.tileNumCol = 6;
    param.paramViewPort.tileNumRow = 3;

    param.usedType = E_MERGE_AND_VIEWPORT;
    void* pI360SCVP = I360SCVP_Init(&param);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return ret;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    printf("pareseOneNalu nalu type=%d size=%d", nal.naluType, nal.dataSize);
    I360SCVP_unInit(pI360SCVP);
    return 0;
}

int processMergeAndViewport() 
{
    int ret = 0;
    param.paramViewPort.faceWidth = 3840;
    param.paramViewPort.faceHeight = 2048;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = 0;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.usedType = E_MERGE_AND_VIEWPORT;
    void* pI360SCVP = I360SCVP_Init(&param);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return ret;
    }

    ret = I360SCVP_process(&param, pI360SCVP);
    if (pOutputFile)
        fwrite(param.pOutputBitstream, 1, param.outputBitstreamLen, pOutputFile);
    printf("I360SCVP_process ret=%d outLen=%d\n", ret, param.outputBitstreamLen);
    I360SCVP_unInit(pI360SCVP);
    return 0;
}

int processStitch() 
{
    int ret = 0;
    param.paramViewPort.faceWidth = 3840;
    param.paramViewPort.faceHeight = 2048;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = 135;
    param.paramViewPort.viewPortPitch = -40;
    param.paramViewPort.viewPortFOVH = 45;
    param.paramViewPort.viewPortFOVV = 45;
    param.usedType = E_STREAM_STITCH_ONLY;
    void* pI360SCVP = I360SCVP_Init(&param);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return ret;
    }

    ret = I360SCVP_process(&param, pI360SCVP);
    if (pOutputFile)
        fwrite(param.pOutputBitstream, 1, param.outputBitstreamLen, pOutputFile);
    printf("I360SCVP_process ret=%d outLen=%d\n", ret, param.outputBitstreamLen);
    I360SCVP_unInit(pI360SCVP);
    return 0;
}

void init2Input()
{
    pInputFile = fopen("../test.265", "rb");
    pInputFileLow = fopen("../test_low.265", "rb");
    if((pInputFile==NULL) ||(pInputFileLow==NULL))
        return;
    frameWidth = 3840;
    frameHeight = 2048;
    frameWidthlow = 1280;
    frameHeightlow = 768;
    bufferlen = frameWidth * frameHeight * 3 / 2;
    bufferlenlow = frameWidthlow * frameHeightlow * 3 / 2;
    memset((void*)&param, 0, sizeof(param_360SCVP));
    pInputBuffer = new unsigned char[bufferlen];
    pInputBufferlow = new unsigned char[bufferlenlow];
    pOutputBuffer = new unsigned char[bufferlen];
    pOutputSEI = new unsigned char[2000];
    if(!pInputBuffer || !pOutputBuffer || !pInputBufferlow || !pOutputSEI)
        return;
    bufferlen = fread(pInputBuffer, 1, bufferlen, pInputFile);
    bufferlenlow = fread(pInputBufferlow, 1, bufferlenlow, pInputFileLow);
    param.pInputBitstream = pInputBuffer;
    param.inputBitstreamLen = bufferlen;
    param.pOutputBitstream = pOutputBuffer;
    param.pInputLowBitstream = pInputBufferlow;
    param.inputLowBistreamLen = bufferlenlow;
    param.frameWidthLow = frameWidthlow;
    param.frameHeightLow = frameHeightlow;
    param.frameWidth = frameWidth;
    param.frameHeight = frameHeight;
    param.pOutputSEI = pOutputSEI;
    param.outputSEILen = 0;
}

void init1Input()
{
    pInputFile = fopen("../test.265", "rb");
    pInputFileLow = NULL;
    if(pInputFile==NULL)
        return;
    frameWidth = 3840;
    frameHeight = 2048;
    frameWidthlow = 1280;
    frameHeightlow = 768;
    bufferlen = frameWidth * frameHeight * 3 / 2;
    bufferlenlow = 0;
    memset((void*)&param, 0, sizeof(param_360SCVP));
    pInputBuffer = new unsigned char[bufferlen];
    pInputBufferlow = NULL;
    pOutputBuffer = new unsigned char[bufferlen];
    pOutputSEI = new unsigned char[2000];
    if(!pInputBuffer || !pOutputBuffer || !pOutputSEI)
        return;
    bufferlen = fread(pInputBuffer, 1, bufferlen, pInputFile);
    bufferlenlow = 0;
    param.pInputBitstream = pInputBuffer;
    param.inputBitstreamLen = bufferlen;
    param.pOutputBitstream = pOutputBuffer;
    param.pInputLowBitstream = pInputBufferlow;
    param.inputLowBistreamLen = bufferlenlow;
    param.frameWidthLow = frameWidthlow;
    param.frameHeightLow = frameHeightlow;
    param.frameWidth = frameWidth;
    param.frameHeight = frameHeight;
    param.pOutputSEI = pOutputSEI;
    param.outputSEILen = 0;
}

void init()
{
    init1Input();
    pOutputFile = fopen("out.265", "wb+");
}

void deInit() 
{
    if(pInputBuffer)
        delete pInputBuffer;
    pInputBuffer = NULL;
    if(pInputBufferlow)
        delete pInputBufferlow;
    pInputBufferlow = NULL;
    if(pOutputBuffer)
        delete pOutputBuffer;
    pOutputBuffer = NULL;
    if(pOutputSEI)
        delete pOutputSEI;
    pOutputSEI = NULL;
    if (pInputFile)
        fclose(pInputFile);
    pInputFile = NULL;
    if (pInputBufferlow)
        fclose(pInputFileLow);
    pInputFileLow = NULL;
    if (pOutputFile)
        fclose(pOutputFile);
    pOutputFile = NULL;
}
