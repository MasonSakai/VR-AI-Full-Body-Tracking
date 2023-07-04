from subprocess import Popen, PIPE
import time
import threading
import struct

Active = False

Process = None

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
    Process.stdin.write(chr(v).encode('latin1'))
def SendInt32_t(v):
    Process.stdin.write(v.to_bytes(4, 'big'))
    Process.stdin.write(b'\n')
    Process.stdin.flush()
def SendInt16_t(v):
    Process.stdin.write(v.to_bytes(2, 'big'))
    Process.stdin.write(b'\n')
    Process.stdin.flush()
def SendInt8_t(v):
    Process.stdin.write(chr(v).encode('latin1'))
    Process.stdin.write(b'\n')
    Process.stdin.flush()
def SendFloat(v):
    Process.stdin.write(bytearray(struct.pack("f", 5.1)))
    Process.stdin.write(b'\n')
    Process.stdin.flush()

def StartSubprocess(fileName):
    global Process
    Process = Popen([fileName, "HideConsole", "NoVR"], stdout=PIPE, stdin=PIPE, bufsize=-1)
    active = True
def StopProgram():
    active = False
    Process.communicate(chr(127).encode() + b'\n')

def BeginTest(fileName):
    threading.Thread(target=outputLoop, args=(Process.stdout,)).start()
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
