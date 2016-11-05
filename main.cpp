// opencv_p01 - 2011/09/18 - JAS
// Read and display an mat
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cv.hpp>
#include "classes/Image.h"
#include "classes/Geometry.h"

using namespace cv;
using namespace std;

Point2f PERSPECTIVE_DST_POINTS [4] = { Point2f(0,0), Point2f(1000,0),Point2f(1000,1000), Point2f(0,1000)};
Point2f PERSPECTIVE_SRC_POINTS [4];
Image plot;
vector<Vec2i> points;
bool warpedPointsReady = false;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    Image clone = plot.clone();

    if  (event == EVENT_LBUTTONDOWN && points.size() == 8 )
    {
        for(int i = 1; i < points.size(); i += 2){
            Vec4i cLine = Vec4i(points[i-1][0], points[i-1][1], points[i][0], points[i][1]);
            int ii = i == (points.size() - 1) ? 1 : (i+2);
            Vec4i nLine = Vec4i(points[ii-1][0], points[ii-1][1], points[ii][0], points[ii][1]);
            Vec2i intersection =  Geometry::intersect(cLine,nLine);
            PERSPECTIVE_SRC_POINTS[(i-1)/2] = Point(intersection[0],intersection[1]);
        }
        clone.wrapPerspective(PERSPECTIVE_SRC_POINTS, PERSPECTIVE_DST_POINTS);
        warpedPointsReady = true;
        clone.show("Define warp points");

    }
    else {
        if  (event == EVENT_LBUTTONDOWN){
            points.push_back(Vec2i( x, y ));
            if(points.size() == 8) warpedPointsReady = true;
        }
        else if  ( event == EVENT_RBUTTONDOWN )
        {
            if(points.size() > 0)
                points.pop_back();
            warpedPointsReady = false;
        }

        for(int i = 1; i < points.size(); i += 2){
            line(clone.getMat(), Point(points[i-1][0], points[i-1][1]), Point(points[i][0], points[i][1]), Scalar(0,255,0), 2, 8 );
        }
        for(int i = 0; i < points.size(); i++){
            circle(clone.getMat(), Point(points[i][0], points[i][1]), 5,  Scalar(255,255,0), 1, 5, 0 );
        }

        clone.show("Define warp points");
    }
}


int main(int argc, char** argv) {





    // Read empty parking lot image
    //Image background = Image("esguelha.jpg");
    Image background = Image("2012-09-28_05_45_56.jpg");

    //Image im = Image("2012-09-13_07_18_15.jpg");

    //Ask user to define the parking lot edges
    plot = background.clone();

    namedWindow("Define warp points", 1);
    setMouseCallback("Define warp points", CallBackFunc, NULL);
    cout << "--------------------------------------" << endl;
    cout << "Define, by this order: left, top, right and bottom; the parking lot edges. To define each edge, mark to points in the image." << endl;
    cout << "Use the left button to remove the last point" << endl;
    cout << "After defining the 4 edges, use long press to preview. " << endl;
    cout << "Press enter to proceed." << endl;
    do {
        plot.show("Define warp points", 0);
    } while(!warpedPointsReady);
    destroyWindow("Define warp points");




    // Wrap perspective
    //background.wrapPerspective(PERSPECTIVE_SRC_POINTS, PERSPECTIVE_DST_POINTS);

    // Segment spaces/road/trees/walkways
    //background.changeColorSpace(CV_HSV2BGR);
    //Mat eq(background.height(), background.width(), CV_8UC1,Scalar(0,0,0));
    //equalizeHist(background.nthChannel(2,true),eq);

    background.wrapPerspective(PERSPECTIVE_SRC_POINTS, PERSPECTIVE_DST_POINTS).gaussianBlur(9);
    Image plot2 = background.clone();

    background.changeColorSpace(CV_BGR2GRAY).CLAHE();

    background.canny(1000,1300,5, plot);
    vector<Vec4i> lines = background.houghLines();
    vector<Vec4i> horizontalLines, verticalLines;

    for(vector<Vec4i>::iterator it = lines.begin(); it != lines.end(); it++){
        if(Geometry::slopeAngle(*it) == 90){
            horizontalLines.push_back(*it);
            line(plot2.getMat(), Point((*it)[0], (*it)[1]), Point((*it)[2], (*it)[3]), Scalar(0,0,255), 2, 8 );

        }else{
            verticalLines.push_back(*it);
            line(plot2.getMat(), Point((*it)[0], (*it)[1]), Point((*it)[2], (*it)[3]), Scalar(0,255,0), 2, 8 );
            circle( plot2.getMat(), Point( (*it)[0], (*it)[1] ), 5,  Scalar(255,255,0), 1, 5, 0 );
            circle( plot2.getMat(), Point( (*it)[2], (*it)[3] ), 5,  Scalar(255,255,0), 1, 5, 0 );

        }
    }



    for(vector<Vec4i>::iterator it = verticalLines.begin(); it != verticalLines.end(); it++) {
        for (vector<Vec4i>::iterator itt = horizontalLines.begin(); itt != horizontalLines.end(); itt++) {
          Vec2i i =  Geometry::intersect(*it,*itt);
            int xi = i[0], x1 = min((*it)[0],(*it)[0]), x2 = max((*it)[0],(*it)[0]), x3 = min((*itt)[0],(*itt)[0]), x4 = min((*itt)[0],(*itt)[0]);

            if(xi >= x1 && xi <= x2 && xi >= x3 && xi <= x4){
                circle( plot2.getMat(), Point( i[0], i[1] ), 5,  Scalar(255,0,0), 1, 5, 0 );
            }
        }
    }



    //Create a window
    namedWindow("Segmented lots", 1);
    //setMouseCallback("Original", CallBackFunc, NULL);
    plot2.show("Segmented lots", 0);


    return 0;

}