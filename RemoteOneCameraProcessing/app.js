'use strict';
var path = require('path');
var express = require('express');

var app = express();

var staticPath = path.join(__dirname, '/');
app.use(express.static(staticPath));

// Allows you to set port in the project properties.
app.set('port', process.env.PORT || 3000);

var server = app.listen(app.get('port'), function () {
    console.log('listening yeet');
});

const model = poseDetection.SupportedModels.MoveNet;
const detector = await poseDetection.createDetector(model);

//const poses = await detector.estimatePoses(image);