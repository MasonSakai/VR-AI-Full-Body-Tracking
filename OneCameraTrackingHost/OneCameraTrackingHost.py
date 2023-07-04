import OneCameraTrackingSubprocess as OCTSubprocess
from OneCameraTrackingSubprocess import SendPreCode, SendInt32_t, SendInt16_t, SendInt8_t, SendFloat, StartSubprocess, StopProgram
import time
import threading

FileName = r"..\x64\Debug\OneCameraTracking.exe"

print("Starting...")
StartSubprocess(FileName)
print("Started")

OCTSubprocess.BeginTest(FileName)

print("Stopping...")
StopProgram()
print("Stopped")
