# This python script automates the commands of using sox to 
# convert raw to wav file format


import os
import subprocess

def convert_raw_to_wav(raw_filename, wav_filename, sample_rate=44100, bit_depth=16, channels=1):
    # Check for existing wav file to remove
    if os.path.exists(wav_filename):
        os.remove(wav_filename)
        print(f"Deleted existing file: {wav_filename}")
        
    # Create sox command for automation
    command = [
        "sox",
        # Set the sample rate
        "-r", str(sample_rate), 
        # Set the encoding type
        "-e", "signed-integer",
        # Set the bit depth
        "-b", str(bit_depth),
        # Set the number of channels
        "-c", str(channels),
        # Specify the raw and wav files
        raw_filename,
        wav_filename
    ]
    
    try:
        result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"Conversion successful! {raw_filename} -> {wav_filename}")
    except subprocess.CalledProcessError as e:
        print(f"Error during conversion: {e.stderr.decode()}")
        
if __name__ == "__main__":
    raw_file = "RECORD.RAW"
    wav_file = "RECORD.wav"
    
    if os.path.exists(raw_file):
        convert_raw_to_wav(raw_file, wav_file)
    else:
        print(f"Error: {raw_file} does not exist.")
    