from subprocess import Popen, PIPE
import threading
import struct

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
    Process.stdin.write(bytearray(struct.pack("f", 5.1)))
    Process.stdin.flush()
def SendCode(v):
    SendInt8_t(v);

def StartSubprocess(fileName):
    global Process
    Process = Popen([fileName, "NoVR", "NoBreak"], stdout=PIPE, stdin=PIPE)
    active = True
def StopProgram():
    active = False
    Process.communicate(chr(127).encode())
   

def WaitForInit():
    stdout = Process.stdout
    while(True):
            text = stdout.read(1);
            if(text == b'\x00'):
                return True
            elif(text == b'\x01'):
                return False
            print(text.decode('latin1'), end='')

def outputLoop(stdout):
    print("Starting Thread...")
    while(True):
        try:
            text = stdout.read1().decode('latin1')
            print(text, end='')
        except Exception as e:
            break
    print("Thread Ending")

def BeginPrint():
    threading.Thread(target=outputLoop, args=(Process.stdout,)).start()

def BeginTest():
    BeginPrint()
    time.sleep(2)
    SendCode(136)
    SendInt32_t(27)
    SendCode(137)
    SendInt16_t(68)
    SendCode(138)
    SendInt8_t(100)
    SendCode(139)
    SendFloat(5.1)
    time.sleep(2)
