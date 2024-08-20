//
//  RangeFinder.h
//  llap
//
//  Created by Wei Wang on 2/19/16.
//  Copyright © 2016 Nanjing University. All rights reserved.
//  
// Adapted by Seungyun YEOM, WIT Lab @ KAIST: Conversion to C++ from objective-C

#ifndef RangeFinder_h
#define RangeFinder_h


#include <AudioToolbox/AudioToolbox.h>
#include <Accelerate/Accelerate.h>
#include <libkern/OSAtomic.h>

#include <stdint.h>

//max number of frequency
#define MAX_NUM_FREQS           16
//pi
#define PI                      3.1415926535
//audio sample rate
#define AUDIO_SAMPLE_RATE       48000  //should be the same as in controller, will add later
//default temperature
#define TEMPERATURE             20
//volume
#define VOLUME                  0.2
//cic filter stages
#define CIC_SEC                 4
//cic filter decimation
#define CIC_DEC                 16
//cic filter delay
#define CIC_DELAY               17
//socket buffer length
#define SOCKETBUFLEN            40960
//power threshold
#define POWER_THR               15000
//peak threshold
#define PEAK_THR                220
//dc_trend threshold
#define DC_TREND                0.25

class RangeFinder
{
public:
    RangeFinder( uint32_t inMaxFramesPerSlice, uint32_t inNumFreqs, float inStartFreq, float inFreqInterv );
    ~RangeFinder();

    void processData(float* data, int size);
    float getDistanceChange();
    
    int16_t*        GetPlayBuffer(uint32_t inSamples);
    int16_t*        GetRecDataBuffer(uint32_t inSamples);
    int16_t*         GetOutputData(uint32_t inDataBytes);
    float         GetDistanceChange(void);
    uint8_t*        GetSocketBuffer(void);
    void            AdvanceSocketBuffer(long length);
    uint32_t          mSocBufPos;
    
private:
    void            InitBuffer();
    void            GetBaseBand();
    void            RemoveDC();
    void            SendSocketData();
    float         CalculateDistance();
    
    int maxFramesPerSlice;
    int numFreq;
    float startFreq;
    float freqInterval;
    float distanceChange;

    uint32_t          mNumFreqs;//number of frequency
    uint32_t          mCurPlayPos;//current play position
    uint32_t          mCurProcPos;//current process position
    uint32_t          mCurRecPos;//current receive position
    uint32_t          mLastCICPos;//last cic filter position
    uint32_t          mBufferSize;//buffer size
    uint32_t          mRecDataSize;//receive data size
    uint32_t          mDecsize;//buffer size after decimation
    float         mFreqInterv;//frequency interval
    float         mSoundSpeed;//sound speed
    float         mFreqs[MAX_NUM_FREQS];//frequency of the ultsound signal
    float         mWaveLength[MAX_NUM_FREQS];//wave length of the ultsound signal
    
    int16_t*        mPlayBuffer;
    int16_t*        mRecDataBuffer;
    float*        mFRecDataBuffer;
    float*        mSinBuffer[MAX_NUM_FREQS];
    float*        mCosBuffer[MAX_NUM_FREQS];
    float*        mBaseBandReal[MAX_NUM_FREQS];
    float*        mBaseBandImage[MAX_NUM_FREQS];
    float*        mTempBuffer;
    float*        mCICBuffer[MAX_NUM_FREQS][CIC_SEC][2];
    uint8_t         mSocketBuffer[SOCKETBUFLEN];
    float         mDCValue[2][MAX_NUM_FREQS];
    float         mMaxValue[2][MAX_NUM_FREQS];
    float         mMinValue[2][MAX_NUM_FREQS];
    float         mFreqPower[MAX_NUM_FREQS];
};

#endif /* RangeFinder_h */