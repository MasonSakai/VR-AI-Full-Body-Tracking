import OCTSubprocess

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

class Client:
    """description of class"""

    def __init__(self, index):
        self.index = index

    def onPose(self, data):
        print(self.index, "onPose")
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


