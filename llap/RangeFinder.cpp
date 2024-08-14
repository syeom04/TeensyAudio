#include "ordinaryFunctions.h"
#include "RangeFinder.h"

class RangeFinder {
public:
    Float32 CalculateDistance();

private:
    Float32 mBaseBandReal[MAX_NUM_FREQS][4096];
    Float32 mBaseBandImage[MAX_NUM_FREQS][4096];
    Float32 mDecsize;
    Float32 mNumFreqs;
    Float32 mDCValue[2][MAX_NUM_FREQS];
    Float32 mMinValue[2][MAX_NUM_FREQS];
    Float32 mMaxValue[2][MAX_NUM_FREQS];
    Float32 mWaveLength[MAX_NUM_FREQS];
};

Float32 RangeFinder::CalculateDistance() {
    Float32 distance = 0;
    Complex tempcomplex;
    Float32 tempdata[4096], tempdata2[4096], tempdata3[4096], temp_val;
    Float32 phasedata[MAX_NUM_FREQS][4096];
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
    
    Float32 sumxy = 0;
    Float32 sumy = 0;
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
    
    Float32 deltax = mNumFreqs * ((mDecsize - 1) * mDecsize * (2 * mDecsize - 1) / 6 - (mDecsize - 1) * mDecsize * (mDecsize - 1) / 4);
    Float32 delta = (sumxy - sumy * (mDecsize - 1) / 2.0) / deltax * mNumFreqs / numfreqused;
    
    Float32 varsum = 0;
    Float32 var_val[MAX_NUM_FREQS];
    for (int i = 0; i < mDecsize; i++)
        tempdata2[i]
