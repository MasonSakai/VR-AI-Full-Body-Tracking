from subprocess import Popen, PIPE
import threading
import struct
import sys

Active = False

Process = None

def SendInt32_t(v):
    Process.stdin.write(v.to_bytes(4, 'big'))
    Process.stdin.flush()
def SendInt16_t(v):
    Process.stdin.write(v.to_bytes(2, 'big'))
    Process.stdin.flush()
def SendInt8_t(v):
    Process.stdin.write(chr(v).encode('latin1'))
    Process.stdin.flush()
def SendFloat(v):
    Process.stdin.write(bytearray(struct.pack("f", v)))
    Process.stdin.flush()
def SendCode(v):
    SendInt8_t(v);

def errorOutputLoop():
    stderr = Process.stderr;
    print("Starting Error Thread...")
    while(True):
        try:
            text = stderr.read1();
            print(text.decode('latin1'), end='', file=sys.stderr)
        except Exception as e:
            break
    print("Error Thread Ending")

def StartSubprocess(fileName):
    global Process
    Process = Popen([fileName, "NoBreak", "NoVR"], stdout=PIPE, stdin=PIPE, stderr=PIPE) #"NoVR", 
    active = True
    threading.Thread(target=errorOutputLoop).start()
def StopProgram():
    active = False
    Process.communicate(chr(255).encode('latin1'))

def WaitForInit():
    stdout = Process.stdout
    while(True):
            text = stdout.read(1);
            if(text == b'\x00'):
                return True
            elif(text == b'\x01'):
                return False
            print(text.decode('latin1'), end='')