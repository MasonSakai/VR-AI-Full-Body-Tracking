# VR-AI-Full-Body-Tracking
An AI driven, camera based Full Body tracking solution for VR systems

# Simple explaination
This program, using OpenVR Input Emulator, can create virtual trackers and using cameras placed around the room it can mimic real, vive full body trackers.<br>
Each camera has an associated browser page running a tensorflow pose detection model (AI that can tell where your body is), and the more cameras at different angles the better.<br>
This program features basic Playspace Mover functionality! I need to refine it (and on some systems it only half works?), but it has it and is configurable through the UI.

# Notes
This program practically requires **at least** two cameras, and requires decent placement. Details below.<br>
The AI does not capture 3D position or orientation. That is all done by the program and is prone to issues or inaccuracies.<br>
  Hip tracking side-to-side is not great, as the AI can't capture this movement very well (sorry, no wiggles). Hip tracking is fine otherwise<br>
This may be slow on some hardware, especially since the AI is entirely CPU based and running in a browser (+I've only tested it on chrome)<br>
This has only been tested on windows with chrome, and with Qt may _only_ work on windows<br>
The program (at the time of writing this) is only mildly tested and may be prone to issues. At the moment I don't have access to a headset so I can't do much without feedback.<br>
This program has been developed on the quest 2 over airlink. I do not know the vive controller layout or button masks, so I will need to be given them and will update this later.

# Installation
I don't have any real installer<br>
Just unzip the zip file wherever you want (though it needs to be able to write to config.json)<br>
It **requires** OpenVR Input Emulator, you will likely need this patch for this program to work <br>
https://github.com/Louka3000/OpenVR-InputEmulator-Fixed/releases/tag/v0.1-pimax

# Camera Count and Placement
The program has been set up to allow at max 16 cameras, though this could easily be increased in code<br>
It practically requires at least 2 decently placed cameras, though benifits the more cameras are present<br>
A decently placed camera requires:<br>
 -  A large amount of the playspace viewable<br>
 -  More than likely you'll want a lot of floor visable (since this is for tracking the hip and feet)<br>
 -  Once a camera is calibrated, it should not move, or it will need to be recalibrated again<br>
 -  If a small amount of cameras (2 or 3) are present:<br>
     -  It is preferable to not have them facing directly towards eachother or in the same direction, instead being orthoganal or having a decent angle between them<br>
     -  If they must be on the same wall, place them as far apart as possible<br>
 -  For Calibration:<br>
     -  At least one place in the room that can see the entire body<br>
     -  Can be reached while remaining inside the guardian boundry (at least with oculus, since button inputs aren't read if outside)

If you wish to use your phone as a camera, DroidCam is a good app (this AI will not run on phones, though laptops are good)

# More Details
This program is a cheap solution for full body tracking through OpenVR (Steam VR) and Input Emulator<br>
It uses Pose Detection on multiple cameras in order to track limbs, and will create virtual trackers mimicking these limbs<br>
It can create trackers for ankles, knees, hip, shoulders, and elbows.<br>
Most things can be configured through the UI, website, and config.json file<br>
Each camera runs on a web-browser tab, of which are recommended to be in different windows and all visible, since most browsers **will** slow them down if they are in the background<br>
Not all browsers need to be on the same device! This means a laptop can use it's webcam and give the results of the AI to another computer running the VR. Though for this, at least in chrome, the flag "Insecure origins treated as secure" needs to be set appropriately<br>
