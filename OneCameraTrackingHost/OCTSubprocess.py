from subprocess import Popen, PIPE
import threading
import struct
import sys

Active = False

Process = None

def SendBytes(bytes):
    #print(bytes)
    for b in bytes:
        #split, xxxx---- then ----xxxx
        b1 = (192 + (b >> 4)).to_bytes(1)
        b2 = (192 + (b & 15)).to_bytes(1)
        Process.stdin.write(b1)
        Process.stdin.write(b2)
        #print(b1,b2,sep='',end='');
    #print()
    Process.stdin.flush()
def SendInt32_t(v):
    SendBytes(v.to_bytes(4, 'big'))
def SendInt16_t(v):
    SendBytes(v.to_bytes(2, 'big'))
def SendInt8_t(v):
    SendBytes(chr(v).encode('latin1'))
def SendFloat(v):
    SendBytes(bytearray(struct.pack("f", v)))
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