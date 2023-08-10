# RemoteOneCameraProcessing
This is the website used by the core to run the AI.
This technically doesn't need to be a website, and if you want or need to make your own clients networking details will be provided below




# Network Details
**config**
_get_ and _put_ are the only valid methods

values are:
  autostart (bool)
  confidenceThreshold (float; 0-1)
  cameraName (string)
  If any others are defined they will be provided (with 2 exceptions given below)

_get_ can be provided an index:
  -2 - generates a new config with id `windowconfigs.count()`
  -1 - gets an auto-generated id based on the amount of -1 calls before it, maxing at the highest defined config (default if not given)
  index within defined configs - gets the associated config
  index > defined configs - generates new config and allows that id
Generated configs are made from the last defined config, but aren't written to the global config
  If no configs are defined, one with the properties listed above will be created
All -2 calls will return the same id
Calls will return:
  all config values (above)
  status (string)
  id (int)
  
_put_ will overwrite/append (whichever is applicable) the given config to the global config
This must contain the property id, which signifies where to write it
It is written exactly as is, besides id and status being removed
If the id is above the defined configs, there is no check to see how far, it is simply appended (so be careful)

**SwitchConfig**
recommended _put_
give:
  to (int)
  start (int)
returns:
  same as **config** _get_ with id _to_
_start_ should be the current id
_to_ is the desired id, no checks are performed for overlaps
backend details:
  Has the same effects as **connect** for _to_
  Will disconnect _start,_ so be careful with this

**connect**
recommended _put_
Initializes all camera UI elements
Must be called before **start**
Should be called before **onPoseData**
Does not _need_ to be called until the camera starts
give:
  id (int)

**start**
recommended _put_
Allows the camera to calibrate
give:
  id (int)

**cameraSize**
recommended _put_
Sets the camera's size in pixels
Should be called before **start**
give:
  id (int)
  width (int)
  height (int)

**poseData**
recommended _put_
Gives pose data
Output of AI, filtered out by _confidenceThreshold_ (config)
give:
  id (int)
  pose (dict of pose data)
pose data:
  x (float)
  y (float)
  score (float)
valid pose names are:
  nose
	left_eye
	right_eye
	left_ear
	right_ear
	left_shoulder
	right_shoulder
	left_elbow
	right_elbow
	left_wrist
	right_wrist
	left_hip
	right_hip
	left_knee
	right_knee
	left_ankle
	right_ankle
