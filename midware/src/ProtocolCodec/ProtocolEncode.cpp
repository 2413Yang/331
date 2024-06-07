#include "ProtocolEncode.h"
#include "ProtocolDecode.h"
#include "ProtocolDefine.h"
#include "IdDefine.h"
#include "mlog.h"

namespace ZH
{
    namespace ProtocolCodec
    {

        void CEncoder::frameCfg(void)
        {
            addFrameDef(DOWN_MAJOR_INIT, {});
            addFrameDef(DOWN_MINOR_ACK, {1, 1, 1, 1, 1, 1, 1, 1,
                                         1, 1, 1, 1, 1, 1, 1, 1,
                                         1, 1, 1, 1, 1, 1, 1, 1,
										 1, 1, 6});
            addFrameDef(DOWN_MAJOR_SET, {8, 8});
            addFrameDef(DOWN_MINOR_SELF_CHECK, {1, 15});
            addFrameDef(DOWN_MINOR_DATA_RESET, {1, 1, 6});
			addFrameDef(DOWN_MINOR_IGN_OFF_POPSID, {8});
			addFrameDef(DOWN_MINOR_IVI_CTRL_BRIGHTNESS, {8});
			addFrameDef(DOWN_MINOR_IGN_STATUS_RESPONSE, {8});
            addFrameDef(DOWN_MINOR_MEMORY_INFO, {2, 2, 1, 2, 1,
												8});
            addFrameDef(DOWN_MAJOR_HEARTBEAT, {16});
            addFrameDef(DOWN_MAJOR_BRIGHTNESS_CTRL, {4, 1, 1, 1, 1});
            addFrameDef(MAJOR_ENTER_UPGRADE_MODE, {8});
            addFrameDef(DOWN_MAJOR_RTC_SET, {8, 8, 8, 8, 8, 8, 8});
            addFrameDef(DOWN_MINOR_IVI_INTERACTIVE, {8,8,8,8,8,8,8,8});
            addFrameDef(DOWN_MINOR_IVI_OTA, {8,8,8,8,8,8,8,8});
            addFrameDef(DOWN_MINOR_IVI_SOUND, {8,8,8,8,8,8,8,8});
			addFrameDef(DOWN_MINOR_OIL_LIFE_RESET, {8});
#ifdef _LINUX_TARGET_
            addFrameDef(DOWN_MAJOR_SYNC_DATA, {8, 8});
#endif
            // addFrameDef(DOWN_MAJOR_BRIGHTNESS_LEVEL, {8, 8});
            addFrameDef(DOWN_MINOR_MEMORY_INFO_ACK, {2, 2, 1, 2, 1,
												8});
            mStrError = "";
        }

        CEncoder::CEncoder(const uint head)
            : mLength(0), mOffsetLength(0), mMajorID(INVALID_ID),
              mMinorID(INVALID_ID), mFrameHead(head)
        {
            frameCfg();
        }

        void CEncoder::operator()(uint majorID, list_int value, int offset)
        {
            mLength = 0;
            mOffsetLength = offset;
            mMajorID = majorID;
            mMinorID = INVALID_ID;
            setValue(value);
        }
        std::string CEncoder::getFrame(uint majorID, list_int value, int offset)
        {
            std::lock_guard<std::mutex>  lock(mMutex);
            operator()(majorID, value,offset);
            return getFrame();
        }
        void CEncoder::operator()(uint majorID, uint minorID, list_int value, int offset)
        {
            mLength = 0;
            mOffsetLength = offset;
            mMajorID = majorID;
            mMinorID = minorID;
            setValue(value);
        }
        std::string CEncoder::getFrame(uint majorID, uint minorID, list_int value, int offset)
        {
            std::lock_guard<std::mutex>  lock(mMutex);
            operator()(majorID, minorID, value, offset);
            return getFrame();
        }
        std::string CEncoder::getErrorStr()
        {
            std::lock_guard<std::mutex>  lock(mMutex);
            if(mStrError == "")
            {
                return "";
            }
            else
            {
                std::string ret(mStrError);
                mStrError = "";
                return ret;
            }
        }
        uint CEncoder::setValue(list_int value)
        {
            uint ID = INVALID_ID, tmpOffset = 0;
            if (mFrameCfg.find(mMajorID) != mFrameCfg.end())
                ID = mMajorID;
            else if (mFrameCfg.find(mMinorID) != mFrameCfg.end())
                ID = mMinorID;
            else
            {
                char err[128];
                snprintf(err, sizeof(err), "Invalid encoding ID(mMajorID:0x%x,mMinorID:0x%x)", mMajorID,mMinorID);
                mStrError = std::string(mStrError);
                LOGERR(mStrError.c_str());
                return tmpOffset;
            }

            if (value.size() == mFrameCfg[ID].size())
            {
                int j = 0;
                tmpOffset = mOffsetLength;
                for (auto it : mFrameCfg[ID])
                {
                    uint tmpValue = reorder(value[j], it);
                    for (int i = 0; i < it; i++)
                        setBit(tmpOffset + i, (tmpValue >> i) & 0x01);

                    tmpOffset += it;
                    j++;
                }

                if ((tmpOffset % 8) != 0)
                    mLength += (tmpOffset / 8 + 1);
                else
                    mLength += (tmpOffset / 8);
            }
            else
            {
                char err[128];
                snprintf(err, sizeof(err), "encoder majorID: %06X  minorID: %04X  parameter size not match(%d != %d)", mMajorID, mMinorID, value.size(), mFrameCfg[ID].size());
                mStrError = std::string(mStrError);
                LOGERR(mStrError.c_str());
            }

            return tmpOffset;
        }

        std::string CEncoder::getFrame(void)
        {
            unsigned char *frameBuf = getFrameBuf();
            frameBuf[0] = (mFrameHead >> 8) & 0xFF;
            frameBuf[1] = mFrameHead & 0xFF;
            frameBuf[2] = (mMajorID >> 16) & 0xFF;
            frameBuf[3] = (mMajorID >> 8) & 0xFF;
            frameBuf[4] = mMajorID & 0xFF;
            if (mLength == 0)
                mLength = mOffsetLength / 8;
            frameBuf[5] = LCHKSUM((mLength + 2)) << 4; //((mLength) >> 8) & 0xFF;
            frameBuf[6] = (mLength + 2) & 0xFF;
            if (mMinorID != INVALID_ID)
            {
                frameBuf[7] = (mMinorID >> 8) & 0xFF;
                frameBuf[8] = mMinorID & 0xFF;
                frameBuf[9] = (mLength - mOffsetLength / 8) & 0xFF;
                if(frameBuf[9] == 0)
                {
                    char err[128];
                    snprintf(err, sizeof(err), "encoderminorID: %04X len is 0, initSize:%d", mMinorID, mFrameCfg[mMinorID].size());
                    mStrError = std::string(mStrError);
                }
            }

            frameBuf[mLength] = CalcCheckSum((const char *)frameBuf + 2, mLength - 2) & 0xFF;
            frameBuf[mLength + 1] = 0x0A;
            return std::string((char *)frameBuf, mLength + 2);
        }

        StuAckMcu::StuAckMcu(unsigned ID)
        {
#define assignAck(param) ((ID == param) ? (this->id##param = 1) : (this->id##param = 0));

            assignAck(0x600);
            assignAck(0x601);
            assignAck(0x602);
            assignAck(0x603);
            assignAck(0x604);
            assignAck(0x605);
            assignAck(0x606);
            assignAck(0x607);
            assignAck(0x608);
            assignAck(0x609);
            assignAck(0x60A);
            assignAck(0x60B);
            assignAck(0x60C);
            assignAck(0x60D);
            assignAck(0x60E);
            assignAck(0x60F);
            assignAck(0x610);
            assignAck(0x620);
            assignAck(0x611);
            assignAck(0x612);
            assignAck(0x613);
            assignAck(0x614);
            assignAck(0x615);
            assignAck(0x616);
        }

        CIviEncoder::CIviEncoder(unsigned int offset)
            : CEncoder(0xFFBB)
        {
            addFrameDef(IC_TO_IVI_SYNC_TIME, {8, 8, 8, 8, 8, 8, 8});
            addFrameDef(IC_TO_IVI_SYNC_VERSION, {8, 8, 8, 8, 8, 8, 8, 8, 8, 8});
            addFrameDef(IC_TO_IVI_NOTIFY_SOURCE, {8});
            addFrameDef(IC_TO_IVI_REQUEST_PUSH_SCREEN, {8});
            addFrameDef(IC_TO_IVI_NOTIFY_PUSH_SCREEN_STATE, {8});
            addFrameDef(IC_TO_IVI_UPDATE_PACKET_ACK, {8, 8, 8, 8});
            addFrameDef(IC_TO_IVI_UPDATE_FILE_CHECK, {8});
            addFrameDef(IVI_TO_IC_UPDATE_CANCEL_ACK, {8});
            addFrameDef(IC_TO_IVI_ANIMAT_STATE, {8});
            addFrameDef(IC_TO_IVI_360AVM_STATE, {8});
            lockedFrameDef();
        }

        void CIviEncoder::operator()(uint majorID, list_int value, int offset)
        {
            CEncoder::operator()(majorID, value, offset);
        }

        std::string CIviEncoder::getFrame(void)
        {
            unsigned char *frameBuf = getFrameBuf();
            frameBuf[0] = (mFrameHead >> 8) & 0xFF;
            frameBuf[1] = mFrameHead & 0xFF;
            frameBuf[2] = (mMajorID >> 8) & 0xFF;
            frameBuf[3] = mMajorID & 0xFF;
            if (mLength == 0)
                mLength = mOffsetLength / 8;
            frameBuf[4] = ((mLength + 2) >> 8) & 0xFF;
            frameBuf[5] = (mLength + 2) & 0xFF;
            frameBuf[mLength] = CalcCheckSum((const char *)frameBuf + 2, mLength - 2) & 0xFF;
            frameBuf[mLength + 1] = 0x0B;
            return std::string((char *)frameBuf, mLength + 2);
        }

    } // namespace ProtocolCodec
} // namespace ZH