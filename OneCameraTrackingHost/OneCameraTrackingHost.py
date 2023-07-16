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
ConfigFile = r"./config.json"
StaticFiles = {
    '/': '../Remote1CamProcessing/dist/index.html',
    '/main.js': '../Remote1CamProcessing/dist/main.js',
    '/style.css': '../Remote1CamProcessing/dist/style.css',
}

config = {
	"autostart": False,
	"port": 2674,
	"windowConfigs": []
}
sockets = {}

sio = socketio.Server(cors_allowed_origins='*', logger=True, engineio_logger=True)
app = socketio.WSGIApp(sio, static_files=StaticFiles)


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
def OnSpecial(code):
    global sio
    if(code == 17):
        sid = indexQueue.get()
        index = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        sockets[sid] = SocketManager.Client(index);
        sio.emit("config", config["windowConfigs"][index], to=sid)
    elif(code == 18):
        i = int.from_bytes(OCTSubprocess.Process.stdout.read(1), 'big')
        for k in sockets:
            if(sockets[k].index == i):
                sio.emit("requestSize", to=k)
                break
    else:
        print("recieved:",code)


@sio.event
def connect(sid, environ):
    print('connect', sid)
    indexQueue.put(sid)
    OCTSubprocess.SendCode(65)
@sio.event
def disconnect(sid):
    print('disconnect', sid)
    sockets[sid].onDisconnect()
    
@sio.on("config")
def on_config(sid, data):
    if(data == "get"):
        sio.emit("config", config["windowConfigs"][sockets[sid].index], to=sid);
    else: #check if dict?
        print(data)
        config["windowConfigs"][sockets[sid].index] = data
        SaveConfig()
   
@sio.on("start")
def on_start(sid):
    sockets[sid].onStart()
@sio.on("stopped")
def on_stopped(sid):
    sockets[sid].onStop()

@sio.on("initialized")
def on_initialized(sid, data):
    print(sid, "onInitialized", data)
    #if(data == "successful"):
    #    #open next
    #    pass
    #else:
    #    pass

@sio.on("pose")
def on_pose(sid, data):
    if(sid in sockets):
        sockets[sid].onPose(data);

@sio.on("requestSize")
def on_pose(sid, data):
    if(sid in sockets):
        sockets[sid].onSize(data);

def LoadConfig():
    global config
    try:
        with open(ConfigFile, "r") as openfile:
            config = json.load(openfile);
            print(config)
            openfile.close()
    except Exception as e:
        print(e)
def SaveConfig():
    print(config)
    jObj = json.dumps(config, indent=4)
    with open(ConfigFile, "w") as outfile:
        outfile.write(jObj)
        outfile.close()

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


print("Starting...")
LoadConfig()
OCTSubprocess.StartSubprocess(FileName)
OCTSubprocess.WaitForInit()
threading.Thread(target=outputLoop).start()
print("Started")

try:
    wsgi.server(eventlet.listen(('', config["port"])), app, log=open(os.devnull,"w"))
except Exception as e:
    print(e, file=sys.stderr)

print("Stopping...")
time.sleep(1)
OCTSubprocess.StopProgram()
print("Stopped")