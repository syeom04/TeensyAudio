import numpy as np
from scipy.fftpack import fft, ifft
from scipy.signal import find_peaks

PI = np.pi
AUDIO_SAMPLE_RATE = 44100
TEMPERATURE = 20  # Default temperature
MAX_NUM_FREQS = 16
CIC_DEC = 8
CIC_SEC = 4
CIC_DELAY = 16
VOLUME = 1
POWER_THR = 0.1
PEAK_THR = 0.1
DC_TREND = 0.1

