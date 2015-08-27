#include <cmath>
#include "x264.hpp"

unsigned int Util264::Ue(char *_pBuf, unsigned int _nLen, unsigned int &_nStartBit)
{
        unsigned int nZeroNum = 0;
        while (_nStartBit < _nLen * 8) {
                if (_pBuf[_nStartBit / 8] & (0x80 >> (_nStartBit % 8))) {
                        break;
                }
                nZeroNum++;
                _nStartBit++;
        }
	_nStartBit ++;

        uint32_t dwRet = 0;
        for (unsigned int i = 0; i < nZeroNum; i++) {
                dwRet <<= 1;
                if (_pBuf[_nStartBit / 8] & (0x80 >> (_nStartBit % 8))) {
                        dwRet += 1;
                }
                _nStartBit++;
        }
        return (1 << nZeroNum) - 1 + dwRet;
}

int Util264::Se(char *_pBuf, unsigned int _nLen, unsigned int &_nStartBit)
{
        int UeVal = Ue(_pBuf, _nLen, _nStartBit);
        double k = UeVal;
        int nValue = ceil(k / 2);
        
	if (UeVal % 2 == 0) {
                nValue =- nValue;
	}
        return nValue;
}


uint32_t Util264::u(unsigned int _nBitCount, char * _pBuf, unsigned int &_nStartBit)
{
        uint32_t dwRet = 0;
        
	for (unsigned int i = 0; i < _nBitCount; i++) {
                dwRet <<= 1;
                if (_pBuf[_nStartBit / 8] & (0x80 >> (_nStartBit % 8))) {
                        dwRet += 1;
                }
                _nStartBit++;
        }
        return dwRet;
}

bool Util264::h264_decode_sps(char * _pBuf,unsigned int _nLen,int &_width,int &_height)
{
	unsigned int startBit = 0;
	int forbiddenZeroBit = u(1, _pBuf, startBit);
	int nalRefIdc = u(2, _pBuf, startBit);
	int nalUnitType = u(5, _pBuf, startBit);

	if (nalUnitType == 0x7) {
		int profileIdc = u(8, _pBuf, startBit);
		int constraintSet0Flag = u(1, _pBuf, startBit);
		int constraintSet1Flag = u(1, _pBuf, startBit);
		int constraintSet2Flag = u(1, _pBuf, startBit);
		int constraintSet3Flag = u(1, _pBuf, startBit);
		int reservedZero4bits = u(4, _pBuf, startBit);
		int levelIdc = u(8, _pBuf, startBit);

		int seqParamSetId = Ue(_pBuf, _nLen, startBit);

		if (profileIdc == 100 || profileIdc == 110 || profileIdc == 122 || profileIdc == 144) {
			int chromaFormatIdc = Ue(_pBuf, _nLen, startBit);
			if (chromaFormatIdc == 0x3) {
				int residualColorTransformFlag = u(1, _pBuf, startBit);
			}
			int bitDepthLumaMinus8 = Ue(_pBuf, _nLen, startBit);
			int bitDepthChromaMinus8 = Ue(_pBuf, _nLen, startBit);
			int qpprimeYZeroTransformBypassFlag = u(1, _pBuf, startBit);
			int seqScalingMatrixPresentFlag = u(1, _pBuf, startBit);

			int seqScalingListPresentFlag[8];
			if (seqScalingMatrixPresentFlag) {
				for (int i = 0; i < 8; i++) {
					seqScalingListPresentFlag[i] = u(1, _pBuf, startBit);
				}
			}
		}
		int log2MaxFrameNumMinus4 = Ue(_pBuf, _nLen, startBit);
		int picOrderCntType = Ue(_pBuf, _nLen, startBit);
		if (picOrderCntType == 0) {
			int log2MaxPicOrderCntLsbMinus4 = Ue(_pBuf, _nLen, startBit);
		} else if (picOrderCntType == 1) {
			int deltaPicOrderAlwaysZeroFlag = u(1, _pBuf, startBit);
			int offsetForNonRefPic = Se(_pBuf, _nLen, startBit);
			int offsetForTopToBottomField = Se(_pBuf, _nLen, startBit);
			int numRefFramesInPicOrderCntCycle = Ue(_pBuf, _nLen, startBit);
			
			int *offsetForRefFrame = new int[numRefFramesInPicOrderCntCycle];
			for (int i = 0; i < numRefFramesInPicOrderCntCycle; i++) {
				offsetForRefFrame[i] = Se(_pBuf, _nLen, startBit);
			}
			delete[] offsetForRefFrame;
		}
		int numRefFrames = Ue(_pBuf, _nLen, startBit);
		int gapsInFrameNumValueAllowedFlag = u(1, _pBuf, startBit);
		int picWidthInMbsMinus1 = Ue(_pBuf, _nLen, startBit);
		int picHeightInMapUnitsMinus1 = Ue(_pBuf, _nLen, startBit);

		_width = (picWidthInMbsMinus1 + 1) * 16;
		_height = (picHeightInMapUnitsMinus1 + 1) * 16;

		return true;
	} else {
		return false;
	}
	return true;
}
