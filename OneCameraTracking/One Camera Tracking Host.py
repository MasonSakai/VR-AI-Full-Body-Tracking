from subprocess import Popen, PIPE
import time
import sys
import threading

active = True

def outputLoop(stdout):
    print("Starting Thread...")
    while(True):
        try:
            text = stdout.read1().decode('latin1')
            print(text, end='', flush=True)
        except Exception as e:
            break
    print("Thread Ending")

FileName = r"C:\Users\Mason Sakai\source\repos\VR AI Full Body Tracking\x64\Debug\OneCameraTracking.exe"

with Popen([FileName, "HideConsole", "NoVR"], stdout=PIPE, stdin=PIPE, bufsize=-1) as p:
    print("Starting...")
    threading.Thread(target=outputLoop, args=(p.stdout,)).start()
    time.sleep(2)
    p.stdin.write(chr(5).encode())
    p.stdin.write((27).to_bytes(4, 'big'))
    p.stdin.write(b'\n')
    p.stdin.flush()
    time.sleep(8)
    print("Stopping...")
    active = False
    p.communicate(chr(27).encode() + b'\n')
    print("Stopped")
