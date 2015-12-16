#ifndef __LZMA_UTIL_H
#define __LZMA_UTIL_H

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif
//
//SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
//    UInt64 unpackSize);
//SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream);
//SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize/*, char *rs*/);

SRes LzmaDecFile(unsigned char *dest, const unsigned char *src, ICompressProgress *progress);
SRes LzmaEncFile(unsigned char *dest, const unsigned char *src, ICompressProgress *progress);

#ifdef __cplusplus
}
#endif
#endif