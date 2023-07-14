import OCTSubprocess

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
        OCTSubprocess.SendCode(68)
        OCTSubprocess.SendInt8_t(self.index)
        OCTSubprocess.SendInt16_t(data["width"])
        OCTSubprocess.SendInt16_t(data["height"])

    def onDisconnect(self):
        OCTSubprocess.SendCode(66)
        OCTSubprocess.SendCode(self.index);


