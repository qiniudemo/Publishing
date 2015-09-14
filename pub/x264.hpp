#ifndef __X264_HPP__
#define __X264_HPP__

typedef unsigned int uint32_t;

class Util264
{
public:
	static bool h264_decode_sps(char * buf,unsigned int nLen,int &width,int &height);
private:
	static unsigned int Ue(char *_pBuf, unsigned int _nLen, unsigned int &_nStartBit);
	static int Se(char *_pBuf, unsigned int _nLen, unsigned int &_nStartBit);
	static uint32_t u(unsigned int _nBitCount, char * _pBuf, unsigned int &_nStartBit);
};

#endif
