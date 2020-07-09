//
// Created by Administrator on 2020/7/8.
//

#include "hevcframespliter.h"
#include "../../360SCVPBitstream.h"
#include "../../360SCVPHevcParser.h"
#include "../../360SCVPCommonDef.h"
#define MAX_BUFF_SIZE 2000000
#define MAX_PARAM_SIZE 10000
HevcFrameSplitter::HevcFrameSplitter(const char *fileName)
{
    mInputFile = fopen(fileName, "rb+");
    mInputBuffer.size = 0;
    mInputBuffer.buf = new uint8_t [MAX_BUFF_SIZE];
    mOutputBuffer.size = 0;
    mOutputBuffer.buf = new uint8_t [MAX_BUFF_SIZE];
    mParameterSets.size = 0;
    mParameterSets.buf = new uint8_t [MAX_PARAM_SIZE];
    mNalBuffer.size = 0;
    mNalBuffer.buf = new uint8_t[MAX_BUFF_SIZE];
}

HevcFrameSplitter::~HevcFrameSplitter()
{
    if (mInputFile)
    {
        fclose(mInputFile);
        mInputFile = nullptr;
    }
    if (mInputBuffer.buf)
    {
        delete mInputBuffer.buf;
        mInputBuffer.buf = nullptr;
        mInputBuffer.size = 0;
    }
    if (mOutputBuffer.buf)
    {
        delete mOutputBuffer.buf;
        mOutputBuffer.buf = nullptr;
        mOutputBuffer.size = 0;
    }
    if (mParameterSets.buf)
    {
        delete mParameterSets.buf;
        mParameterSets.buf = nullptr;
        mParameterSets.size = 0;
    }
    if (mNalBuffer.buf)
    {
        delete mNalBuffer.buf;
        mNalBuffer.buf = nullptr;
        mNalBuffer.size = 0;
    }
}

int copyData(HevcBuf* srcBuf, HevcBuf *dstBuf, uint32_t maxBufSize)
{
    if (srcBuf->size + dstBuf->size + 4 > maxBufSize)
    {
        printf("[splitter]copyData srcBuf->size + dstBuf->size + 4 > maxBufSize\n");
        return -1;
    }
    uint8_t startCodeBuf[4] = {0,0,0,1};
    memcpy(dstBuf->buf + dstBuf->size, startCodeBuf, 4);
    dstBuf->size += 4;
    memcpy(dstBuf->buf + dstBuf->size, srcBuf->buf, srcBuf->size);
    dstBuf->size += srcBuf->size;
    return 0;
}

int HevcFrameSplitter::getOneFrame(HevcBuf *hevcBuf)
{
    if (mInputFile == nullptr)
    {
        printf("[splitter]getOneFrame file==null\n");
        return -1;
    }
    int ret = fread(mInputBuffer.buf, 1, MAX_BUFF_SIZE, mInputFile);
    if (ret <= 0)
    {
        printf("[splitter]getOneFrame fread ret<=0\n");
        return -1;
    }
    mInputBuffer.size = ret;
    GTS_BitStream *bs = gts_bs_new((int8_t*)mInputBuffer.buf, mInputBuffer.size, GTS_BITSTREAM_READ);
    if (bs == nullptr)
    {
        printf("[splitter]getOneFrame gts_bs_new=null\n");
        return -1;
    }

    int firstFlagCount = 0;
    uint32_t nal_size;
    while (gts_bs_available(bs))
    {
        uint8_t nal_unit_type;
        uint64_t nal_start, start_code;

        start_code = gts_bs_read_U32(bs);
        if (start_code >> 8 == 0x000001)
        {
            nal_start = gts_bs_get_position(bs) - 1;
            gts_bs_seek(bs, nal_start);
            start_code = 1;
        }

        if (start_code != 0x00000001)
        {
            gts_bs_del(bs);
            ret = -1;
            break;
        }

        nal_start = gts_bs_get_position(bs);
        nal_size = gts_media_nalu_next_start_code_bs(bs);
        if (nal_start + nal_size > mInputBuffer.size)
        {
            gts_bs_del(bs);
            ret = -1;
            break;
        }

        if (nal_size > MAX_BUFF_SIZE)
        {
            printf("[splitter]getOneFrame nal_size > mNalBuffer.size\n");
            ret = -1;
            break;
        }
        gts_bs_read_data(bs, (int8_t*)mNalBuffer.buf, nal_size);
        mNalBuffer.size = nal_size;
        hevc_specialInfo specialInfo;
        HEVCState hevc;
        gts_media_hevc_parse_nalu(&specialInfo, (int8_t*)mNalBuffer.buf, nal_size, &hevc);

        nal_unit_type = specialInfo.naluType;

        if (hevc.s_info.first_slice_segment_in_pic_flag == 1)
        {
            firstFlagCount++;
            if (firstFlagCount == 2)
            {

                printf("[splitter]getOneFrame firstFlagCount == 2\n");
                ret = 0;
                break;
            }
        }

        if (specialInfo.layer_id)
        {
            ret = -1;
            break;
        }

        switch (nal_unit_type)
        {
            case GTS_HEVC_NALU_SEQ_PARAM:
            case GTS_HEVC_NALU_PIC_PARAM:
            case GTS_HEVC_NALU_VID_PARAM:
                if(copyData(&mNalBuffer, &mParameterSets, MAX_PARAM_SIZE) != 0)
                {
                    printf("[splitter]getOneFrame copy nal type=%d error", nal_unit_type);
                    ret = -1;
                    break;
                }
            case GTS_HEVC_NALU_ACCESS_UNIT:
                /*slice_segment_layer_rbsp*/
            case GTS_HEVC_NALU_SLICE_TRAIL_N:
            case GTS_HEVC_NALU_SLICE_TRAIL_R:
            case GTS_HEVC_NALU_SLICE_TSA_N:
            case GTS_HEVC_NALU_SLICE_TSA_R:
            case GTS_HEVC_NALU_SLICE_STSA_N:
            case GTS_HEVC_NALU_SLICE_STSA_R:
            case GTS_HEVC_NALU_SLICE_BLA_W_LP:
            case GTS_HEVC_NALU_SLICE_BLA_W_DLP:
            case GTS_HEVC_NALU_SLICE_BLA_N_LP:
            case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
            case GTS_HEVC_NALU_SLICE_IDR_N_LP:
            case GTS_HEVC_NALU_SLICE_CRA:
            case GTS_HEVC_NALU_SLICE_RADL_N:
            case GTS_HEVC_NALU_SLICE_RADL_R:
            case GTS_HEVC_NALU_SLICE_RASL_N:
            case GTS_HEVC_NALU_SLICE_RASL_R:
            case GTS_HEVC_NALU_PREFIX_SEI:
                if(copyData(&mNalBuffer, &mOutputBuffer, MAX_BUFF_SIZE) != 0)
                {
                    printf("[splitter]getOneFrame copy nal type=%d error", nal_unit_type);
                    ret = -1;
                    break;
                }
                break;
            default:
                break;
        }
        if (ret == -1) 
        {
            break;
        }
    }
    uint64_t pos = gts_bs_get_position(bs);
    if (pos < mInputBuffer.size)
    {
        fseek(mInputFile, pos-mInputBuffer.size, SEEK_CUR);
    }
    gts_bs_del(bs);
    if (ret == -1)
    {
        return -1;
    }
    else
    {
        hevcBuf->buf = mOutputBuffer.buf;
        hevcBuf->size = mOutputBuffer.size;
        return 0;
    }

}
