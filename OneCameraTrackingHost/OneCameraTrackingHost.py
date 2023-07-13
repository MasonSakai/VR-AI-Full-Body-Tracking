import OCTSubprocess
import SocketManager
from eventlet import wsgi
import eventlet
import socketio
import time
import threading

FileName = r"..\x64\Debug\OneCameraTracking.exe"

sockets = {}
socketIndex = 0

sio = socketio.Server(cors_allowed_origins='*')
app = socketio.WSGIApp(sio)

@sio.event
def connect(sid, environ):
    print('connect ', sid)
    global sockets, socketIndex
    sockets[sid] = SocketManager.Client(socketIndex);
    socketIndex = socketIndex + 1

@sio.on("pose")
def on_pose(sid, data):
    global sockets
    sockets[sid].onPose(data);

@sio.event
def disconnect(sid):
    print('disconnect ', sid)
    global sockets
    sockets[sid].onDisconnect()

print("Starting...")
OCTSubprocess.StartSubprocess(FileName)
OCTSubprocess.WaitForInit()
OCTSubprocess.BeginPrint()
print("Started")

wsgi.server(eventlet.listen(('', 2673)), app)

print("Stopping...")
OCTSubprocess.StopProgram()
print("Stopped")