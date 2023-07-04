from subprocess import Popen, PIPE
import time
import threading
import struct

active = False

p = None

def outputLoop(stdout):
    print("Starting Thread...")
    while(True):
        try:
            text = stdout.read1().decode('latin1')
            print(text, end='', flush=True)
        except Exception as e:
            break
    print("Thread Ending")

def SendPreCode(v):
    p.stdin.write(chr(v).encode('latin1'))
def SendInt32_t(v):
    p.stdin.write(v.to_bytes(4, 'big'))
    p.stdin.write(b'\n')
    p.stdin.flush()
def SendInt16_t(v):
    p.stdin.write(v.to_bytes(2, 'big'))
    p.stdin.write(b'\n')
    p.stdin.flush()
def SendInt8_t(v):
    p.stdin.write(chr(v).encode('latin1'))
    p.stdin.write(b'\n')
    p.stdin.flush()
def SendFloat(v):
    p.stdin.write(bytearray(struct.pack("f", 5.1)))
    p.stdin.write(b'\n')
    p.stdin.flush()

def StartSubprocess(fileName):
    global p
    p = Popen([fileName, "HideConsole", "NoVR"], stdout=PIPE, stdin=PIPE, bufsize=-1)
    active = True
def StopProgram():
    active = False
    p.communicate(chr(127).encode() + b'\n')

def BeginTest(fileName):
    threading.Thread(target=outputLoop, args=(p.stdout,)).start()
    time.sleep(2)
    SendPreCode(136)
    SendInt32_t(27)
    SendPreCode(137)
    SendInt16_t(68)
    SendPreCode(138)
    SendInt8_t(100)
    SendPreCode(139)
    SendFloat(5.1)
    time.sleep(2)
