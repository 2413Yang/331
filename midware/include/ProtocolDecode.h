#ifndef PROTOCOL_DECODE_H
#define PROTOCOL_DECODE_H

#include <string>
#include <vector>

#include "DataDefine.h"
#include "ProtocolDefine.h"

namespace ZH
{
    namespace ProtocolCodec
    {
        class CDecoder : public CProtocolFrame
        {
        public:
            CDecoder();
            virtual ~CDecoder();
            unsigned int operator[](int sub);
            void operator()(std::string &frame, int offset=PROTOCOL_BASE_LEN_BIT);
            void operator()(unsigned char *frame, int size);
			const std::vector<unsigned int> getVecData() {return mResult;}

        protected:
            virtual void decode(int offset=PROTOCOL_BASE_LEN_BIT);

            std::vector<unsigned int> mResult;
        };

        class CIviDecoder : public CDecoder
        {
        public:
            CIviDecoder();
            virtual ~CIviDecoder();
            virtual unsigned int getMajorID(void);
            unsigned char *getData(void);

        private:
            virtual void decode(void);
        };
    }

}

#endif