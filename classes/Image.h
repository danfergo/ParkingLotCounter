//
// Created by danfergo on 17-10-2016.
//

#ifndef DISPLAYIMAGE_IMAGE_H
#define DISPLAYIMAGE_IMAGE_H

#include <iostream>
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

class Image {
private:
    Mat mat;
public:
    Image(Mat image);
    Image(string fileName);
    Image(int width, int height, Scalar color);

    const Mat &getMat() const;
    void addSaltAndPepperNoise(int percentage);
    int width();
    int height();
    Image & setMat(Mat image);
    Image ();
    Image plotNthChannelHistogram(int channel, Scalar color = Scalar(255, 255, 255), int with = 768, int height = 256);


    Image & wrapPerspective(Point2f from [4], Point2f to [4]);
    Image & changeColorSpace(int conversionCode);
    Image & nthChannel(int channel, bool grayMode = true);
    Image & addToChannel(int channel, int quantity);
    Image & sobel();
    Image & gaussianBlur(int kernelSize, double sigma = 0);
    Image & canny(int lowTreshold = 150, int highTreshold = 200, int kernelSize = 3, Image & plotIn = NONE_IMAGE);
    vector<Vec4i> houghLines(int channel = 0);
    Image & plotHoughLines(Image &plot, int channel = 0);
    Image & threshold();
    Image & equalizeHist();
    Image & CLAHE(float clipLimit = 2.0, int size = 8, int channel = -1);

    Image clone();
    Image canvas(int color);
    void show(string title, int waitKey = -1);


private:
    static Image NONE_IMAGE;
    static void plotHistogramsIn(Mat image, Mat & canvas, vector<Scalar> colors, vector<int> channelsIdx, int thickness = 1);
    static void plotHistogramIn(Mat image, Mat & canvas, Scalar color, int channelIdx, int thickness =1 );
    static void fullSobel(Mat image, Mat & dst);
};


#endif //DISPLAYIMAGE_IMAGE_H
