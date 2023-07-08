folder = r"C:\Users\Mason Sakai\source\repos\VR AI Full Body Tracking\RemoteOneCameraProcessing"

with open(folder+r"\out.txt", 'r') as fp:
    maxLen = 0
    maxLines = []
    for line in fp:
        myLen = len(line)
        if(myLen >= 180):
            print(len(line),": ",line, sep = "")
        if(myLen > maxLen):
            maxLen = myLen
            maxLines.clear()
            maxLines.append(line)
        elif(myLen == maxLen):
            maxLines.append(line)
    print("Base Directory Length:", len(folder))
    print("Max Length:",maxLen)
    print("Delta:", maxLen - len(folder))
    print("File(s) are: ")
    for line in maxLines:
        print(line)
    input("Press Enter to close...")
            
