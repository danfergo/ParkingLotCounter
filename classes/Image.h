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
    /**
    Matrix containing the details of the image
    **/
    Mat mat;

public:

    /**
    Default constructor
    **/
    Image();

    /**
    Constructor of Image with a Mat image.
     @param image Matrix that is going to be used
    **/
    Image(Mat image);

    /**
    Constructor of Image with a filename.
     @param filename String containing the location of the file relative to the current directory
    **/
    Image(string fileName);

    /**
    Constructor of Image with a constant color.
     @param width Width of the image
     @param height Height of the image
     @param color Color to use on the image (0<=color<=255)
    **/
    Image(int width, int height, Scalar color);

    /**
    Get the width of the current Image
    **/
    int width();

    /**
    Get the height of the current Image
    **/
    int height();

    /**
    Return the address of the matrix of this Image
    **/
    const Mat &getMat() const;

    /**
    Set the matrix to be used
     @param image Matrix that is going to be used
    **/
    Image &setMat(Mat image);

    /**
    Add Salt and Pepper Noise to the current Image
     @param percentage Percentage of the image that is going to be changed
    **/
    void addSaltAndPepperNoise(int percentage);

    /**
    Plot the chosen channel Histogram
     @param channel Channel to be plotted
     @param color Colors used in the histogram. Default: Scalar(255, 255, 255)
     @param with Width of the window. Default: 768
     @param height Height of the window. Default: 256
    **/
    Image plotNthChannelHistogram(int channel, Scalar color = Scalar(255, 255, 255), int with = 768, int height = 256);

    /**
    Wrap the perspective of the current image
     @param from Points of the image defining the area to be warped
     @param to Points defining where to warp to inside a 1000x1000 Image
    **/
    Image &wrapPerspective(Point2f from[4], Point2f to[4]);

    /**
    Change the color space of the current image
     @param conversionCode color space conversion code (see cv::ColorConversionCodes).
    **/
    Image &changeColorSpace(int conversionCode);

    /**
    Wrap the perspective of the current image
     @param from Points of the image defining the area to be warped
     @param to Points defining where to warp to inside a 1000x1000 Image
    **/
    Image &nthChannel(int channel, bool grayMode = true);

    /**
    Adds a certain quantity to a certain channel
     @param channel Channel to be changed
     @param quantity Quantity to be added
    **/
    Image &addToChannel(int channel, int quantity);

    /**
    Applies the Sobel algorithm to the current image //TODO
    **/
    Image &sobel();

    /**
    Blurs an image using a Gaussian filter
     @param kernelSize Gaussian kernel size. Must be positive and odd.
     @param sigma Gaussian kernel standard deviation in the X and Y directions. Default: 0
    **/
    Image &gaussianBlur(int kernelSize, double sigma = 0);

    /**
    Finds edges in the current image using the Canny algorithm
      @param lowThreshold Low threshold for the hysteresis procedure. Default: 150
      @param highThreshold High threshold for the hysteresis procedure. Default: 200
      @param apertureSize aperture size for the Sobel operator. Default: 3
      @deprecated
      @param plotIn
    **/
    Image &canny(int lowThreshold = 150, int highThreshold = 200, int apertureSize = 3, Image &plotIn = NONE_IMAGE);

    /**
    Finds line segments in the image using the probabilistic Hough transform.
     @param channel Channel used. Default: 0
    **/
    vector<Vec4i> houghLines(int channel = 0);

    /**
    Plot the lines found using the Hough transform.
     @param plot Image used to plot the lines
     @param channel Channel used. Default: 0
    **/
    Image &plotHoughLines(Image &plot, int channel = 0);

    /**
    Applies a fixed-level threshold using the type THRESH_BINARY_INV with the values 100 and 255 on the current Image
    **/
    Image &threshold();

    /**
    Equalizes the histogram of a grayscale image.
    **/
    Image &equalizeHist();

    /**
    Applies the CLAHE (Contrast Limited Adapatative Histogram Equalization) on the Image
     @param clipLimit Contrast limit. Default: 2.0
     @param size size of the tiles. Default: 8
     @param channel Channel used. If <0 apply on the image. If >=0 apply on the specified channel. Default: -1
    **/
    Image &CLAHE(float clipLimit = 2.0, int size = 8, int channel = -1);

    /**
    Get a clone of this Image
    **/
    Image clone();

    /**
    Get canvas with a certain color
     @param color Color of the canvas (0<=color<=255)
    **/
    Image canvas(int color);

    /**
    Shows the current Image
     @param title Title of the window
     @param waitKey Specify if the image waits for a key to continue. <1: don't wait; =0: wait for a key; >0: wait for 'waitKey' milliseconds
    **/
    void show(string title, int waitKey = -1);

private:
    /**
    Image without content
    **/
    static Image NONE_IMAGE;

    /**
    //TODO
     @param image
     @param canvas
     @param colors
     @param channelsIdx
     @param thickness
    **/
    static void
    plotHistogramsIn(Mat image, Mat &canvas, vector<Scalar> colors, vector<int> channelsIdx, int thickness = 1);

    /**
    //TODO
     @param image
     @param canvas
     @param colors
     @param channelsIdx
     @param thickness
    **/
    static void plotHistogramIn(Mat image, Mat &canvas, Scalar color, int channelIdx, int thickness = 1);

    /**
    Applies the Sobel algorithm to an image //TODO
     @param image Input matrix were algorithm was used.
     @param destination Destination matrix.
    **/
    static void fullSobel(Mat image, Mat &destination);
};


#endif //DISPLAYIMAGE_IMAGE_H
