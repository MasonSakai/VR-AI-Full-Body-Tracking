import OCTSubprocess
import SocketManager
from eventlet import wsgi
import eventlet
import socketio
import time
import threading
import queue
import os
import sys
import json

FileName = r"..\x64\Debug\OneCameraTracking.exe"
StaticFiles = {
    '/': '../Remote1CamProcessing/dist/index.html',
    '/main.js': '../Remote1CamProcessing/dist/main.js',
    '/style.css': '../Remote1CamProcessing/dist/style.css',
}

sockets = {}

sio = socketio.Server(cors_allowed_origins='*')
app = socketio.WSGIApp(sio, static_files=StaticFiles)

indexQueue = queue.Queue()
#  0 reserved (for empty bytes)
#  4 reserved (eof indicator)
# 10 reserved (\n)
# 13 reserved (\r)
# 17 Request Index Return
# 18 Request Size
# 26 reserved (eof indicator)
def OnSpecial(code):
    if(code == 17):
        sid = indexQueue.get()
        index = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        sockets[sid] = SocketManager.Client(index);
        sio.emit("config", {}, to=sid);
    elif(code == 18):
        i = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        for k in sockets:
            if(sockets[k].index == i):
                sio.emit("request size", to=k)
                return
    else:
        print("recieved:",code)


@sio.event
def connect(sid, environ):
    print('connect ', sid)
    indexQueue.put(sid)
    OCTSubprocess.SendCode(65)
@sio.event
def disconnect(sid):
    print('disconnect ', sid)
    global sockets
    sockets[sid].onDisconnect()
    
@sio.on("config")
def on_config(sid, data):
    if(data == "get"):
        #get
        pass
    else: #check if dict?
        #tryset
        pass
    
@sio.on("initialized")
def on_initialized(sid, data):
    if(data == "successful"):
        #open next
        pass
    else:
        #tryset
        pass

@sio.on("pose")
def on_pose(sid, data):
    global sockets
    if(sid in sockets):
        sockets[sid].onPose(data);

@sio.on("request size")
def on_pose(sid, data):
    global sockets
    if(sid in sockets):
        sockets[sid].onSize(data);


print("Starting...")
OCTSubprocess.StartSubprocess(FileName)
OCTSubprocess.WaitForInit()


def outputLoop():
    print("Starting stdout Thread...")
    stdout = OCTSubprocess.Process.stdout
    try:
        while(True):
            text = stdout.read(1);
            if(text == b''):
                continue
            i = int.from_bytes(text, 'big')
            if(i == 0):
                continue
            if(i < 32 and i != 10 and i != 13):
                OnSpecial(i)
            else:
                print(text.decode('latin1'), end='')
    except Exception as e:
        print(e, file=sys.stderr)
    print("stdout Thread Ending")

threading.Thread(target=outputLoop).start()
print("Started")

try:
    wsgi.server(eventlet.listen(('', 2673)), app, log=open(os.devnull,"w"))
except Exception as e:
    print(e, file=sys.stderr)

print("Stopping...")
time.sleep(5)
OCTSubprocess.StopProgram()
print("Stopped")