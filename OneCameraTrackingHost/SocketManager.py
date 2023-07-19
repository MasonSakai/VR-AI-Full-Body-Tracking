import OCTSubprocess
from eventlet import wsgi
import eventlet
import socketio
import sys
import os
import queue
import __main__
from __main__ import config

#Move all socketio processes here
#separate socketio and webhosting components, it may work again
#figure out how to close the server without ctrl+c
#For the time being, revert it; I will deal with this later

PoseDict = [
    "nose",
    "left_eye",
    "right_eye",
    "left_ear",
    "right_ear",
    "left_shoulder",
    "right_shoulder",
    "left_elbow",
    "right_elbow",
    "left_wrist",
    "right_wrist",
    "left_hip",
    "right_hip",
    "left_knee",
    "right_knee",
    "left_ankle",
    "right_ankle"
]

StaticFiles = {
    '/': '../Remote1CamProcessing/dist/index.html',
    '/main.js': '../Remote1CamProcessing/dist/main.js',
    '/style.css': '../Remote1CamProcessing/dist/style.css',
}

sockets = {}

sio = socketio.Server(cors_allowed_origins='*')#, logger=True, engineio_logger=True)
app = socketio.WSGIApp(sio, static_files=StaticFiles)

def StartServer():
    try:
        wsgi.server(eventlet.listen(('', config["hostport"])), app, log=open(os.devnull,"w"))
    except Exception as e:
        print(e, file=sys.stderr)

@sio.event
def connect(sid, environ):
    print('connect', sid)
    indexQueue.put(sid)
    OCTSubprocess.SendCode(65)
@sio.event
def disconnect(sid):
    print('disconnect', sid)
    sockets[sid].onDisconnect()
   
@sio.on("start")
def on_start(sid):
    sockets[sid].onStart()
@sio.on("stopped")
def on_stopped(sid):
    sockets[sid].onStop()

@sio.on("pose")
def on_pose(sid, data):
    if(sid in sockets):
        sockets[sid].onPose(data);

@sio.on("requestSize")
def on_pose(sid, data):
    if(sid in sockets):
        sockets[sid].onSize(data);



def RequestCalibration(index):
    OCTSubprocess.SendCode(67)
    OCTSubprocess.SendInt8_t(index)

indexQueue = queue.Queue()
#  0 reserved (for empty bytes)
#  4 reserved (eof indicator)
# 10 reserved (\n)
# 13 reserved (\r)
# 17 Request Index Return
# 18 Request Size
# 26 reserved (eof indicator)
def OnSpecial(code): #figure out why this doesn't work
    global sio
    if(code == 17):
        sid = indexQueue.get()
        index = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        sockets[sid] = Client(index)
        sockets[sid].onStart()
        #sio.emit("config", config["windowConfigs"][0], to=sid)
    elif(code == 18):
        i = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        for k in sockets:
            if(sockets[k].index == i):
                sio.emit("requestSize", to=k)
                break
    else:
        print("recieved:",code)

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


class Client:
    """description of class"""

    def __init__(self, index):
        self.index = index

    def onPose(self, data):
        #print("onPose")
        OCTSubprocess.SendCode(64)
        OCTSubprocess.SendCode(self.index)
        for i in range(0, len(PoseDict)):
            name = PoseDict[i];
            if name in data:
                OCTSubprocess.SendCode(0);
                OCTSubprocess.SendFloat(data[name]['x']);
                OCTSubprocess.SendFloat(data[name]['y']);
                OCTSubprocess.SendFloat(data[name]['score']);
            else:
                OCTSubprocess.SendCode(128);

    def onSize(self, data):
        print(self.index, "onSize")
        OCTSubprocess.SendCode(68)
        OCTSubprocess.SendInt8_t(self.index)
        OCTSubprocess.SendInt16_t(data["width"])
        OCTSubprocess.SendInt16_t(data["height"])

    def onDisconnect(self):
        OCTSubprocess.SendCode(66)
        OCTSubprocess.SendCode(self.index);

    def onStart(self):
        print(self.index, "onStart")
        OCTSubprocess.SendCode(69)
        OCTSubprocess.SendInt8_t(self.index)
    def onStop(self):
        print(self.index, "onStop")
        OCTSubprocess.SendCode(70)
        OCTSubprocess.SendInt8_t(self.index)


