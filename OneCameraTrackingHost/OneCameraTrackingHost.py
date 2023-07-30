import OCTSubprocess
import time
import threading
import os
import sys
import json

FileName = r"..\x64\Debug\Tracking Driver.exe"
ConfigFile = r"../Remote1CamProcessing/config.json"

config = {
	"autostart": False,
	"port": 2674,
	"hostport": 2673,
	"windowConfigs": []
}

import SocketManager
from SocketManager import sockets



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
        


print("Starting...")
LoadConfig()
OCTSubprocess.StartSubprocess(FileName)
OCTSubprocess.WaitForInit()
threading.Thread(target=SocketManager.outputLoop).start()
print("Started")

SocketManager.StartServer()

print("Stopping...")
time.sleep(1)
OCTSubprocess.StopProgram()
time.sleep(5)
print("Stopped")