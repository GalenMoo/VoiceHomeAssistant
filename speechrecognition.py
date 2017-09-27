# -*- coding: utf-8 -*-
# @Author: Galen Wu
# @Date:   2017-06-07 01:10:41
# @Last Modified by:   GalenMoo
# @Last Modified time: 2017-06-09 13:10:59
import math
import time
import serial
import speech_recognition as sr
import matplotlib.pyplot as plt
from os import path
import wave
import struct

# This script uses Python 2.7 and takes in audio data
# through the usbmodemM4321001 serial terminal and transmit
# a 'N' and 'F' if the user speaks the correct keyword 

# void -> time sample array
# collects the ADC data from the board to an array
def collect_data():
    ser = serial.Serial('/dev/tty.usbmodemM4321001', 115200, timeout = 1)
    x = []
    print("start speaking")
    for i in range(46800): # 46800 = 1 second sample
        inputChar = ser.read()
        if (inputChar == b''):
            continue
        x.append(ord(inputChar))
    time.sleep(0.1)
    ser.close()
    print("stop speaking")
    return x

# time sample array -> wav file
# records the sample wav file
def record(x):
    output = wave.open('input.wav', 'w')
    output.setparams((1, 1, 46800, 0, 'NONE', 'noncompressed'))
    data = []
    for i in range(len(x)):
        packeddata = struct.pack('h', x[i])
        data.append(packeddata)
        data.append(packeddata)

    value_str = b''.join(data)
    output.writeframes(value_str)
    output.close()

# time sample array -> plot
# plots the sound wave
def plot(x):
    plt.plot(x)
    plt.ylabel('some numbers')
    plt.show()

# void -> str
# reads the input file and returns the words
def compare():
    AUDIO_FILE = path.join(path.dirname(path.realpath(__file__)), "input.wav")

    # # use the audio file as the audio source
    r = sr.Recognizer()
    with sr.AudioFile(AUDIO_FILE) as source:
        audio = r.record(source)  # read the entire audio file

    # # recognize speech using Google Speech Recognition
    try:
        # for testing purposes, we're just using the default API key
        # to use another API key, use `r.recognize_google(audio, key="GOOGLE_SPEECH_RECOGNITION_API_KEY")`
        # instead of `r.recognize_google(audio)`
        recognizedwords = r.recognize_google(audio)
        print "did you say:", recognizedwords
    except sr.UnknownValueError:
        print("Could not understand audio")
    except sr.RequestError as e:
        print("Could not request results from Google Speech Recognition service; {0}".format(e))
    return recognizedwords

# str -> void
# transmit commands based on a keyword
def transmit(keyword):
    if (keyword == "on"):
        ser = serial.Serial('/dev/tty.usbmodemM4321001', 115200, timeout = 1)
        print("Turning on light")
        ser.write('N')
        ser.close()
    elif (keyword == "off"):
        ser = serial.Serial('/dev/tty.usbmodemM4321001', 115200, timeout = 1)
        print("Turning off light")
        ser.write('F')
        ser.close()

def main():
    data = collect_data()   # collects data
    record(data)            # writes to a wav file
    result = compare()      # returns spoken word
    
    if (result == "on"):
        transmit("on")
    elif (result == "off"):
        transmit("off")
    else:
        print("Not a valid word")

if __name__ == "__main__":
    main()