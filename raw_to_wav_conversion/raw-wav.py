import wave
import numpy as np

def raw_to_wav(raw_filename, wav_filename, sample_rate=44100, bit_depth=16, channels=1):
    # Determine sample width in bytes
    sample_width = bit_depth // 8
    
    # Read raw audio data
    with open(raw_filename, 'rb') as raw_file:
        raw_data = raw_file.read()
    
    # Convert raw data to numpy array
    # The dtype depends on the bit depth (e.g., 'int16' for 16-bit)
    data = np.frombuffer(raw_data, dtype=np.int16)
    
    # If mono, no reshaping is necessary
    # Write WAV file
    with wave.open(wav_filename, 'wb') as wav_file:
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(sample_width)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(data.tobytes())

# Example usage
raw_to_wav('RECORD.RAW', 'RECORD.wav', sample_rate=44100, bit_depth=16, channels=1)
