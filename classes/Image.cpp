//
// Created by danfergo on 17-10-2016.
//

#include <opencv2/imgcodecs.hpp>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cv.hpp>
#include "Image.h"

Image Image::NONE_IMAGE = Image(Mat());

Image::Image(string fileName): mat(imread(fileName)) {

}

Image::Image(int width, int height, Scalar color): mat(Mat(height, width, CV_8UC3, color)) {
}

const Mat &Image::getMat() const {
    return mat;
}

void Image::addSaltAndPepperNoise(int percentage) {
    srand (time(NULL));

    int nChannels = mat.channels();
    for(int i = 0; i < mat.rows; i++){
        for(int j = 0; j < mat.cols; j++ ){
            int noise = rand() % 100;
            for(int c = 0; c < nChannels; c++){
                if(noise <= percentage/2){
                    mat.at<uchar>(j,i*nChannels+c) = 255;
                }else if(noise <= percentage){
                    mat.at<uchar>(j,i*nChannels+c) = 0;
                }
            }
        }
    }
}

Image & Image::nthChannel(int channel, bool grayMode) {
    Mat zeros = Mat::zeros(mat.cols, mat.rows, CV_8UC1);
    std::vector<cv::Mat> planes;
    cv::split(mat, planes);

    if(grayMode){
        return setMat(planes[channel]);
    }

    Mat channelMat;
    Mat channels[] =  {zeros, zeros, zeros};
    channels[channel] = planes[channel];
    merge(channels, 3, channelMat);
    return setMat(channelMat);
}

Image & Image::addToChannel(int channel, int quantity) {
    Mat zeros = Mat::zeros(mat.cols, mat.rows, CV_8UC1);
    Mat delta = Mat(mat.cols, mat.rows, CV_8UC1, Scalar(quantity));


    Mat filterMat;
    Mat channels[] =  {zeros, zeros, zeros};
    channels[channel] = delta;
    merge(channels, 3, filterMat);

    return setMat(mat + filterMat);
}

void Image::plotHistogramsIn(Mat image, Mat &canvas, vector<Scalar> colors, vector<int> channelsIdx, int thickness) {

    if(channelsIdx.size() == 0){
       for(int c = 0; c < image.channels(); c++)  channelsIdx.push_back(c);
    }

    // Separate the mat in 3 places ( B, G and R )
    vector<Mat> bgr_planes;
    split(image, bgr_planes);

    /// Establish the number of bins
    int histSize = 256;

    /// Set the ranges ( for B,G,R) )
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    /// Compute the histograms:
    vector<Mat> hists;
    for(int c = 0; c < channelsIdx.size(); c++){
        Mat hist;
        calcHist( &bgr_planes[channelsIdx[c]], 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );
        hists.push_back(hist);
    }

    int bin_w = cvRound( (double) canvas.cols/histSize );


    /// Normalize the result to [ 0, histImage.rows ]
    for(int c = 0; c < channelsIdx.size(); c++){
        normalize(hists[c], hists[c], 0, canvas.rows, NORM_MINMAX, -1, Mat() );
    }

    /// Draw for each channel
    for( int i = 1; i < histSize; i++ )
    {
        for(int c = 0; c < channelsIdx.size(); c++){
            line( canvas, Point( bin_w*(i-1), canvas.rows - cvRound(hists[c].at<float>(i-1)) ) ,
                  Point( bin_w*(i), canvas.rows - cvRound(hists[c].at<float>(i)) ),
                  colors[c], thickness, 8, 0  );
        }
    }
}

int Image::width() {
    return mat.cols;
}

int Image::height() {
    return mat.rows;
}

void Image::fullSobel(Mat image, Mat &dst) {
    Mat grayScale(image.cols,image.rows, CV_8UC3);
    if(image.channels() == 1){
        grayScale = image;
    }else{
        cvtColor(image, grayScale, CV_BGR2GRAY);
    }
    GaussianBlur( grayScale, grayScale, Size(3,3), 0, 0, BORDER_DEFAULT );

    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;

    /// Generate grad_x and grad_y
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;

    /// Gradient X
    //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
    Sobel(grayScale, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_x, abs_grad_x );

    /// Gradient Y
    //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
    Sobel(grayScale, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_y, abs_grad_y );

    /// Total Gradient (approximate)
    addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst );
}

void Image::plotHistogramIn(Mat image, Mat &canvas, Scalar color, int channelIdx, int thickness) {
    vector<Scalar> colors;
    colors.push_back(color);

    vector<int> channelsIdx;
    channelsIdx.push_back(channelIdx);

    return Image::plotHistogramsIn(image, canvas, colors, channelsIdx, thickness);
}

Image & Image::setMat(Mat image) {
    this->mat = image;
    return *this;
}

Image & Image::wrapPerspective(Point2f *from, Point2f *to) {
    Mat tmp;
    Mat transform = getPerspectiveTransform(from, to);
    warpPerspective(mat, tmp, transform, Size(1000,1000));
    return setMat(tmp);
}

Image & Image::changeColorSpace(int conversionCode) {
    Mat tmp;
    cvtColor(mat, tmp, conversionCode);
    return setMat(tmp);
}

Image Image::plotNthChannelHistogram(int channel, Scalar color, int with, int height) {
    Mat plot(256, 768, CV_8UC3, Scalar(0,0,0));
    Image::plotHistogramIn(nthChannel(channel, true).mat, plot, Scalar(255,255,255), 0);
    return Image(plot);
}

Image::Image(Mat img): mat(img) {
}

Image & Image::sobel() {
    Mat mat(mat.rows, mat.cols, CV_8UC1, Scalar(0));
    Image::fullSobel(mat,mat);
    return setMat(mat);
}

Image & Image::plotHoughLines(Image &plot, int channel) {
    vector<Mat> channels;
    split(mat, channels);
    vector<Vec4i> lines;
    HoughLinesP(channels[channel], lines, CV_PI/4, CV_PI/2, 20, 50, 50);
    for( size_t i = 0; i < lines.size(); i++ )
    {
        line(plot.getMat(), Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 1, 8 );
    }

    return plot;
}

void Image::show(string title, int wk) {
    imshow(title, mat);
    if(wk >=0){
        waitKey(wk);
    }
}

Image & Image::threshold() {
    Mat mat(mat.rows, mat.cols, CV_8UC1, Scalar(0));
    cv::threshold(mat, mat, 100, 255, 1);
    return setMat(mat);
}

Image Image::clone() {
    return Image(mat.clone());
}

Image & Image::canny(int lowTreshold, int highTreshold, int kernelSize, Image & plotIn) {
    cv::Canny(mat, mat, lowTreshold, highTreshold, kernelSize);
    if(&plotIn != &NONE_IMAGE){
       // add(mat, plotIn.getMat(),plotIn.getMat());
    }
    return *this;
}

Image &Image::equalizeHist() {
    cv::equalizeHist(mat, mat);
    return *this;
}

Image &Image::CLAHE(float clipLimit, int s, int channel) {
    if(channel < 0){
        createCLAHE(clipLimit,Size(s,s))->apply(mat,mat);
    }else{
        vector<Mat> mats;
        split(mat, mats);
        createCLAHE(clipLimit,Size(s,s))->apply(mats[channel],mats[channel]);
        merge(mats, mat);
    }
    return *this;
}

Image Image::canvas(int color){
    Mat m = cv::Mat(mat.rows, mat.cols, CV_8UC1, Scalar(color));

    return Image(m);
}

Image &Image::gaussianBlur(int kernelSize, double sigma) {

    GaussianBlur(mat,mat,Size(kernelSize, kernelSize), sigma);
    return *this;
}

vector<Vec4i> Image::houghLines(int channel) {
    vector<Mat> channels;
    split(mat, channels);
    vector<Vec4i> lines;
    HoughLinesP(channels[channel], lines, 1, CV_PI/2, 20, 50, 50);
    return lines;
}

Image::Image(): mat(Mat()) {

}

