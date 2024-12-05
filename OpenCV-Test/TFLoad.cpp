#include "TFLoad.h"
#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace dnn;

Net net;

void GetModel() {
    // Load the model
    net = readNetFromONNX("C:\\Users\\User\\source\\repos\\VR-AI-Full-Body-Tracking\\Models\\onnx\\yolo11n-pose.onnx");
}

void RunModel(Mat image) {
    

    // Prepare the input
    //cv:: = cv::imread("path/to/image.jpg");
    Mat blob = blobFromImage(image, 1.0, cv::Size(640, 480));

    // Set the input blob
    net.setInput(blob);

    // Forward pass
    Mat output = net.forward();

    int classId;
    double confidence;
    minMaxIdx(output, NULL, &confidence, NULL, &classId);

    std::cout << "Class ID: " << classId << ", Confidence: " << confidence << std::endl;
}