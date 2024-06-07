#ifndef PROTOCOL_ENCODE_H
#define PROTOCOL_ENCODE_H

#include <string>

#include "DataDefine.h"
#include "ProtocolDefine.h"
#include <mutex>

namespace ZH
{
    namespace ProtocolCodec
    {
#define LCHKBITS(LEN, BIT) ((uint8_t)((LEN >> BIT) & 0X0F))
#define LCHKSUM(LEN) ((uint8_t)(((~(LCHKBITS(LEN, 8) + LCHKBITS(LEN, 4) + LCHKBITS(LEN, 0))) + 1) & 0X0F))
        using uint = unsigned int;
        using list_int = std::vector<int>;

        class CEncoder : public CProtocolFrame
        {

        public:
            CEncoder(const uint head = 0xFFAA);
            void frameCfg(void);

            virtual void operator()(uint majorID, list_int value, int offset = MAJOR_ID_OFFSET);
            virtual void operator()(uint majorID, uint minorID, list_int value, int offset = MINOR_ID_OFFSET);

            virtual std::string getFrame(void);

            virtual std::string getFrame(uint majorID, list_int value, int offset = MAJOR_ID_OFFSET);
            virtual std::string getFrame(uint majorID, uint minorID, list_int value, int offset = MINOR_ID_OFFSET);
            std::string getErrorStr();

        public:
            uint setValue(list_int value);

        protected:
            uint mLength, mOffsetLength, mMajorID, mMinorID;
            const uint mFrameHead;
            std::mutex  mMutex;
            std::string mStrError;
        };

        struct StuAckMcu
        {
            StuAckMcu(unsigned ID);
            int id0x600 : 1;
            int id0x601 : 1;
            int id0x602 : 1;
            int id0x603 : 1;
            int id0x604 : 1;
            int id0x605 : 1;
            int id0x606 : 1;
            int id0x607 : 1;
            int id0x608 : 1;
            int id0x609 : 1;
            int id0x60A : 1;
            int id0x60B : 1;
            int id0x60C : 1;
            int id0x60D : 1;
            int id0x60E : 1;
            int id0x60F : 1;
            int id0x610 : 1;
            int id0x620 : 1;
            int id0x611 : 1;
            int id0x612 : 1;
            int id0x613 : 1;
            int id0x614 : 1;
            int id0x615 : 1;
            int id0x616 : 1;
			int id0x617 : 1;
			int id0x618 : 1;
        };

        class CIviEncoder : public CEncoder
        {

        public:
            CIviEncoder(unsigned int offset = 48);
            virtual void operator()(uint majorID, list_int value, int offset = IVI_MAJOR_ID_OFFSET);

            virtual std::string getFrame(void);
            // virtual std::string getFrame(const unsigned int majorID){}
        };
    }
}

#endif