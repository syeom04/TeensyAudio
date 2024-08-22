import matplotlib.pyplot as plt
from scipy.io import wavfile
import os
import numpy as np

PATH = '/teensy/TeensyAudio/signal_analysis/'
CURRENT_FILE = 'RECORD.wav'
sample_rate, samples = wavfile.read(os.path.join(PATH, CURRENT_FILE))

# If the audio is stereo (2 channels), convert it to mono by averaging the two channels
if len(samples.shape) == 2:
    samples = np.mean(samples, axis=1)

sample_rate = 4410
plt.figure(figsize=(10, 6))
plt.specgram(samples, Fs=sample_rate, NFFT=1024, noverlap=512, cmap='viridis')
plt.title('Spectrogram of {}'.format(CURRENT_FILE))
plt.xlabel('Time [s]')
plt.ylabel('Frequency [Hz]')
plt.colorbar(label='Intensity [dB]')
plt.show()
