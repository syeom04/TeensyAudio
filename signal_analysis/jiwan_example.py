import matplotlib.pyplot as plt
from scipy import signal
from scipy.io import wavfile
import os

PATH = '/teensy/TeensyAudio/signal_analysis/'
CURRENT_FILE = 'RECORD.wav'
sample_rate, samples = wavfile.read(os.path.join(PATH, CURRENT_FILE))
plt.specgram(samples, Fs=sample_rate)
plt.show()