import os

folder = os.getcwd()

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
    print("Raw Length Without Directory:", maxLen - len(folder))
    print("With Reserved:",maxLen+80)
    if(maxLen+80 > 260):
        print("Path is too long (with reserve > 260)")
    print("File(s) are: ")
    for line in maxLines:
        print(line)
    input("Press Enter to close...")
            
