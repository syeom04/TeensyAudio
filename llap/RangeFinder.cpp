#include "ordinaryFunctions.h"
#include "RangeFinder.h"
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <numeric>

RangeFinder::RangeFinder(uint32_t inMaxFramesPerSlice, uint32_t inNumFreq, float inStartFreq, float inFreqInterv) {
    // Number of frequency
    mNumFreqs = inNumFreq;
    // Buffer size
    mBufferSize = inMaxFramesPerSlice;
    // Frequency interval
    mFreqInterv = inFreqInterv;
    // Receive data size
    mRecDataSize = 4 * inMaxFramesPerSlice;
    // Sound speed
    mSoundSpeed = 331.3 + 0.606 * TEMPERATURE;
    // Init buffer
    for (uint32_t i = 0; i < MAX_NUM_FREQS; i++) {
        mSinBuffer[i] = (float*) calloc(2 * inMaxFramesPerSlice, sizeof(float));
        mCosBuffer[i] = (float*) calloc(2 * inMaxFramesPerSlice, sizeof(float));

        mFreqs[i] = inStartFreq + i * inFreqInterv;

        mWaveLength[i] = mSoundSpeed / mFreqs[i] * 1000; // all distance is in mm

        mBaseBandReal[i] = (float*) calloc(mRecDataSize / CIC_DEC, sizeof(float));
        mBaseBandImage[i] = (float*) calloc(mRecDataSize / CIC_DEC, sizeof(float));
        for (uint32_t k = 0; k < CIC_SEC; k++) {
            mCICBuffer[i][k][0] = (float*) calloc(mRecDataSize / CIC_DEC + CIC_DELAY, sizeof(float));
            mCICBuffer[i][k][1] = (float*) calloc(mRecDataSize / CIC_DEC + CIC_DELAY, sizeof(float));
        }
    }

    mPlayBuffer = (int16_t*) calloc(2 * inMaxFramesPerSlice, sizeof(int16_t));

    mRecDataBuffer = (int16_t*) calloc(mRecDataSize, sizeof(int16_t));
    mFRecDataBuffer = (float*) calloc(mRecDataSize, sizeof(float));
    mTempBuffer = (float*) calloc(mRecDataSize, sizeof(float));
    mCurPlayPos = 0;
    mCurRecPos = 0;
    mCurProcPos = 0;
    mLastCICPos = 0;
    mDecsize = 0;
    mSocBufPos = 0;

    InitBuffer();
}

void RangeFinder::InitBuffer() {
    for (uint32_t i = 0; i < mNumFreqs; i++) {
        for (uint32_t n = 0; n < mBufferSize * 2; n++) {
            mCosBuffer[i][n] = cos(2 * PI * n / AUDIO_SAMPLE_RATE * mFreqs[i]);
            mSinBuffer[i][n] = -sin(2 * PI * n / AUDIO_SAMPLE_RATE * mFreqs[i]);
        }
        mDCValue[0][i] = 0;
        mMaxValue[0][i] = 0;
        mMinValue[0][i] = 0;
        mDCValue[1][i] = 0;
        mMaxValue[1][i] = 0;
        mMinValue[1][i] = 0;
    }

    float mTempSample;
    for (uint32_t n = 0; n < mBufferSize * 2; n++) {
        mTempSample = 0;
        for (uint32_t i = 0; i < mNumFreqs; i++) {
            mTempSample += mCosBuffer[i][n] * VOLUME;
        }
        mPlayBuffer[n] = static_cast<int16_t>(mTempSample / mNumFreqs * 32767);
    }
}

///////////////////////////////////////////////
// Write the destructor
RangeFinder::~RangeFinder() {
    for (uint32_t i = 0; i < mNumFreqs; i++) {
        if (mSinBuffer[i] != nullptr) {
            free(mSinBuffer[i]);
            mSinBuffer[i] = nullptr;
        }
        if (mCosBuffer[i] != nullptr) {
            free(mCosBuffer[i]);
            mCosBuffer[i] = nullptr;
        }
        if (mBaseBandReal[i] != nullptr) {
            free(mBaseBandReal[i]);
            mBaseBandReal[i] = nullptr;
        }
        if (mBaseBandImage[i] != nullptr) {
            free(mBaseBandImage[i]);
            mBaseBandImage[i] = nullptr;
        }
        for (uint32_t k = 0; k < CIC_SEC; k++) {
            if (mCICBuffer[i][k][0] != nullptr) {
                free(mCICBuffer[i][k][0]);
                mCICBuffer[i][k][0] = nullptr;
            }
            if (mCICBuffer[i][k][1] != nullptr) {
                free(mCICBuffer[i][k][1]);
                mCICBuffer[i][k][1] = nullptr;
            }
        }
    }
    if (mPlayBuffer != nullptr) {
        free(mPlayBuffer);
        mPlayBuffer = nullptr;
    }
    if (mTempBuffer != nullptr) {
        free(mTempBuffer);
        mTempBuffer = nullptr;
    }
    if (mRecDataBuffer != nullptr) {
        free(mRecDataBuffer);
        mRecDataBuffer = nullptr;
    }
    if (mFRecDataBuffer != nullptr) {
        free(mFRecDataBuffer);
        mFRecDataBuffer = nullptr;
    }
}

int16_t* RangeFinder::GetPlayBuffer(uint32_t inSamples) {
    int16_t* playDataPointer = mPlayBuffer + mCurPlayPos;

    mCurPlayPos += inSamples;

    if (mCurPlayPos >= mBufferSize) {
        mCurPlayPos = mCurPlayPos - mBufferSize;
    }

    return playDataPointer;
}

uint8_t* RangeFinder::GetSocketBuffer() {
    return mSocketBuffer;
}

void RangeFinder::AdvanceSocketBuffer(long length) {
    if (length > 0) {
        if (length >= mSocBufPos) {
            mSocBufPos = 0;
            return;
        } else {
            mSocBufPos = mSocBufPos - static_cast<uint32_t>(length);
            memmove(mSocketBuffer, mSocketBuffer + length, mSocBufPos);
            return;
        }
    }
}

int16_t* RangeFinder::GetRecDataBuffer(uint32_t inSamples) {
    int16_t* RecDataPointer = mRecDataBuffer + mCurRecPos;

    mCurRecPos += inSamples;

    if (mCurRecPos >= mRecDataSize) { // overflowed RecBuffer
        mCurRecPos = 0;
        RecDataPointer = mRecDataBuffer;
    }

    return RecDataPointer;
}

int16_t* RangeFinder::GetOutputData(uint32_t inDataBytes) {
    return nullptr;
}

float RangeFinder::GetDistanceChange() {
    float distanceChange = 0.0f;

    // Process the data in the RecDataBuffer and clear the mCurRecPos

    // Get base band signal
    GetBaseBand();

    // Remove DC value from the baseband signal
    RemoveDC();

    // Send baseband signal via socket
    SendSocketData();

    // Calculate distance from the phase change
    distanceChange = CalculateDistance();

    return distanceChange;
}


float RangeFinder::CalculateDistance() {
    float distance = 0;
    Complex tempcomplex;
    float tempdata[4096], tempdata2[4096], tempdata3[4096], temp_val;
    float phasedata[MAX_NUM_FREQS][4096];
    int ignorefreq[MAX_NUM_FREQS];
    
    if (mDecsize > 4096) {
        return 0;
    }

    for (int f = 0; f < mNumFreqs; f++) {
        ignorefreq[f] = 0;
        // Get complex number
        tempcomplex.real = mBaseBandReal[f];
        tempcomplex.imag = mBaseBandImage[f];
        
        // Get magnitude
        calcSquareMag(&tempcomplex, tempdata, mDecsize);
        calculateTotal(tempdata, mDecsize, &temp_val);
        
        if (temp_val / mDecsize > POWER_THR) { // Only calculate the high power vectors
            calculatePhases(&tempcomplex, phasedata[f], mDecsize);
            // Phase unwarp
            for (int i = 1; i < mDecsize; i++) {
                while (phasedata[f][i] - phasedata[f][i - 1] > PI)
                    phasedata[f][i] = phasedata[f][i] - 2 * PI;
                while (phasedata[f][i] - phasedata[f][i - 1] < -PI)
                    phasedata[f][i] = phasedata[f][i] + 2 * PI;
            }
            if (fabs(phasedata[f][mDecsize - 1] - phasedata[f][0]) > PI / 4) {
                for (int i = 0; i <= 1; i++)
                    mDCValue[i][f] = (1 - DC_TREND * 2) * mDCValue[i][f] +
                                     (mMinValue[i][f] + mMaxValue[i][f]) / 2 * DC_TREND * 2;
            }
            
            // Prepare linear regression
            // Remove start phase
            temp_val = -phasedata[f][0];
            addScalarToVector(phasedata[f], temp_val, tempdata, mDecsize);
            // Divide the constants
            temp_val = 2 * PI / mWaveLength[f];
            divideVectorByScalar(tempdata, temp_val, phasedata[f], mDecsize);
        } else { // Ignore the low power vectors
            ignorefreq[f] = 1;
        }
    }
    
    // Linear regression
    for (int i = 0; i < mDecsize; i++)
        tempdata2[i] = i;
    
    float sumxy = 0;
    float sumy = 0;
    int numfreqused = 0;
    for (int f = 0; f < mNumFreqs; f++) {
        if (ignorefreq[f]) {
            continue;
        }
        
        numfreqused++;
        
        multiplyVectors(phasedata[f], tempdata2, tempdata, mDecsize);
        sumVectorElements(tempdata, &temp_val, mDecsize);
        sumxy += temp_val;
        sumVectorElements(phasedata[f], &temp_val, mDecsize);
        sumy += temp_val;
    }
    if (numfreqused == 0) {
        distance = 0;
        return distance;
    }
    
    float deltax = mNumFreqs * ((mDecsize - 1) * mDecsize * (2 * mDecsize - 1) / 6 - (mDecsize - 1) * mDecsize * (mDecsize - 1) / 4);
    float delta = (sumxy - sumy * (mDecsize - 1) / 2.0) / deltax * mNumFreqs / numfreqused;
    
    float varsum = 0;
    float var_val[MAX_NUM_FREQS];
for(int i=0;i<mDecsize;i++)
        tempdata2[i]=i*delta;
    
    //get variance of each freq;
    for(int f=0;f<mNumFreqs;f++)
    {   var_val[f]=0;
        if(ignorefreq[f])
        {
            continue;
        }
        subtractVectors(tempdata2, phasedata[f], tempdata, mDecsize);
        squareVector(tempdata, tempdata3, mDecsize);
        sumVectorElements(tempdata3, &var_val[f], mDecsize);
        varsum+=var_val[f];
    }
    varsum=varsum/numfreqused;
    for(int f=0;f<mNumFreqs;f++)
    {
        if(ignorefreq[f])
        {
            continue;
        }
        if(var_val[f]>varsum)
            ignorefreq[f]=1;
    }
    
    //linear regression
    for(int i=0;i<mDecsize;i++)
        tempdata2[i]=i;
    
    sumxy=0;
    sumy=0;
    numfreqused=0;
    for(int f=0;f<mNumFreqs;f++)
    {
        if(ignorefreq[f])
        {
            continue;
        }
        
        numfreqused++;
        
        multiplyVectors(phasedata[f], tempdata2, tempdata, mDecsize);
        sumVectorElements(tempdata, &temp_val, mDecsize);
        sumxy+=temp_val;
        sumVectorElements(phasedata[f], &temp_val, mDecsize);
        sumy+=temp_val;
        
    }
    if(numfreqused==0)
    {
        distance=0;
        return distance;
    }
    
    delta=(sumxy-sumy*(mDecsize-1)/2.0)/deltax*mNumFreqs/numfreqused;
    
    distance=-delta*mDecsize/2;
    return distance;
}

void RangeFinder::RemoveDC() {
    int f,i;
    float tempdata[4096],tempdata2[4096],temp_val;
    float vsum,dsum,max_valr,min_valr,max_vali,min_vali;

    if (mDecsize > 4096)
        return;

    // 'Levd' algorithm to calculate the DC value;
    for (f = 0; f < mNumFreqs; f++) {
        vsum = 0;
        dsum = 0;

        // real part
        max_valr = findMax(mBaseBandReal[f], mDecsize);
        min_valr = findMin(mBaseBandReal[f], mDecsize);
        varAndSum(mBaseBandReal[f], mDecsize, vsum, dsum);

        // imag part
        max_vali = findMax(mBaseBandImage[f], mDecsize);
        min_vali = findMin(mBaseBandImage[f], mDecsize);
        varAndSum(mBaseBandImage[f], mDecsize, vsum, dsum);

        mFreqPower[f] = vsum + dsum * dsum;

        // Get DC estimation
        if (mFreqPower[f] > POWER_THR) {
            if (max_valr > mMaxValue[0][f] ||
                (max_valr > mMinValue[0][f] + PEAK_THR &&
                 (mMaxValue[0][f] - mMinValue[0][f]) > PEAK_THR * 4)) {
                mMaxValue[0][f] = max_valr;
            }

            if (min_valr < mMinValue[0][f] ||
                (min_valr < mMaxValue[0][f] - PEAK_THR &&
                 (mMaxValue[0][f] - mMinValue[0][f]) > PEAK_THR * 4)) {
                mMinValue[0][f] = min_valr;
            }

            if (max_vali > mMaxValue[1][f] ||
                (max_vali > mMinValue[1][f] + PEAK_THR &&
                 (mMaxValue[1][f] - mMinValue[1][f]) > PEAK_THR * 4)) {
                mMaxValue[1][f] = max_vali;
            }

            if (min_vali < mMinValue[1][f] ||
                (min_vali < mMaxValue[1][f] - PEAK_THR &&
                 (mMaxValue[1][f] - mMinValue[1][f]) > PEAK_THR * 4)) {
                mMinValue[1][f] = min_vali;
            }

            if ((mMaxValue[0][f] - mMinValue[0][f]) > PEAK_THR &&
                (mMaxValue[1][f] - mMinValue[1][f]) > PEAK_THR) {
                for (i = 0; i <= 1; i++)
                    mDCValue[i][f] = (1 - DC_TREND) * mDCValue[i][f] +
                                     (mMinValue[i][f] + mMaxValue[i][f]) / 2 * DC_TREND;
            }
        }

        // remove DC
        for (i = 0; i < mDecsize; i++) {
            mBaseBandReal[f][i] -= mDCValue[0][f];
            mBaseBandImage[f][i] -= mDCValue[1][f];
        }
    }
}

void RangeFinder::SendSocketData() {
    int index;

    // Sending baseband to MATLAB
    index = mSocBufPos;
    for (int i = 0; i < 16; i++) { // number of frequencies
        for (unsigned int k = 0; k < mDecsize; k++) { // iterate through samples
            if (index < SOCKETBUFLEN - 4) { // ensure enough buffer
                mSocketBuffer[index++] = static_cast<uint8_t>(static_cast<short>(mBaseBandReal[i][k]) & 0xFF);
                mSocketBuffer[index++] = static_cast<uint8_t>((static_cast<short>(mBaseBandReal[i][k]) >> 8) & 0xFF);
                mSocketBuffer[index++] = static_cast<uint8_t>(static_cast<short>(mBaseBandImage[i][k]) & 0xFF);
                mSocketBuffer[index++] = static_cast<uint8_t>((static_cast<short>(mBaseBandImage[i][k]) >> 8) & 0xFF);
            }
        }
    }
    mSocBufPos = index - 1;
}

void RangeFinder::GetBaseBand() {
    int i, index, decsize, cid;
    decsize = mCurRecPos / CIC_DEC;
    mDecsize = decsize;

    // Change data from int to float
    for (i = 0; i < mCurRecPos; i++) {
        mFRecDataBuffer[i] = static_cast<float>(mRecDataBuffer[i] / 32767.0);
    }

    for (i = 0; i < mNumFreqs; i++) { // mNumFreqs
        // Multiply the cos
        transformMultiply(mFRecDataBuffer, mCosBuffer[i] + mCurProcPos, mTempBuffer, mCurRecPos);

        cid = 0;
        // Sum CIC_DEC points of data, put into CICbuffer
        memMove(mCICBuffer[i][0][cid], mCICBuffer[i][0][cid] + mLastCICPos, CIC_DELAY);
        index = CIC_DELAY;
        for (uint32_t k = 0; k < mCurRecPos; k += CIC_DEC) {
            mCICBuffer[i][0][cid][index] = accumulate(mTempBuffer + k, CIC_DEC);
            index++;
        }

        // Prepare CIC first level
        memMove(mCICBuffer[i][1][cid], mCICBuffer[i][1][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][0][cid] + CIC_DELAY, mCICBuffer[i][1][cid] + CIC_DELAY, decsize);

        // Prepare CIC second level
        memMove(mCICBuffer[i][2][cid], mCICBuffer[i][2][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][1][cid] + CIC_DELAY, mCICBuffer[i][2][cid] + CIC_DELAY, decsize);

        // Prepare CIC third level
        memMove(mCICBuffer[i][3][cid], mCICBuffer[i][3][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][2][cid] + CIC_DELAY, mCICBuffer[i][3][cid] + CIC_DELAY, decsize);

        // CIC last level to Baseband
        partialSum(mCICBuffer[i][3][cid] + CIC_DELAY, mBaseBandReal[i], decsize);

        // Multiply the sin
        transformMultiply(mFRecDataBuffer, mSinBuffer[i] + mCurProcPos, mTempBuffer, mCurRecPos);
        
        cid = 1;
        // Sum CIC_DEC points of data, put into CICbuffer
        memMove(mCICBuffer[i][0][cid], mCICBuffer[i][0][cid] + mLastCICPos, CIC_DELAY);
        index = CIC_DELAY;
        for (uint32_t k = 0; k < mCurRecPos; k += CIC_DEC) {
            mCICBuffer[i][0][cid][index] = accumulate(mTempBuffer + k, CIC_DEC);
            index++;
        }

        // Prepare CIC first level
        memMove(mCICBuffer[i][1][cid], mCICBuffer[i][1][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][0][cid] + CIC_DELAY, mCICBuffer[i][1][cid] + CIC_DELAY, decsize);

        // Prepare CIC second level
        memMove(mCICBuffer[i][2][cid], mCICBuffer[i][2][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][1][cid] + CIC_DELAY, mCICBuffer[i][2][cid] + CIC_DELAY, decsize);

        // Prepare CIC third level
        memMove(mCICBuffer[i][3][cid], mCICBuffer[i][3][cid] + mLastCICPos, CIC_DELAY);
        // Sliding window sum
        partialSum(mCICBuffer[i][2][cid] + CIC_DELAY, mCICBuffer[i][3][cid] + CIC_DELAY, decsize);

        // CIC last level to Baseband
        partialSum(mCICBuffer[i][3][cid] + CIC_DELAY, mBaseBandImage[i], decsize);
    }

    mCurProcPos = mCurProcPos + mCurRecPos;
    if (mCurProcPos >= mBufferSize) {
        mCurProcPos = mCurProcPos - mBufferSize;
    }
    mLastCICPos = decsize;
    mCurRecPos = 0;
}


void RangeFinder::processData(float* data, int size) {
    distanceChange = std::accumulate(data, data+size, 0.0f) / size;
}

float RangeFinder::getDistanceChange() {
    return distanceChange;
}


