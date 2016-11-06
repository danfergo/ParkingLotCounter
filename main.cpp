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

    if  (event == EVENT_LBUTTONDOWN && points.size() >= 8 )
    {
        clone.wrapPerspective(PERSPECTIVE_SRC_POINTS, PERSPECTIVE_DST_POINTS);
        clone.show("Define warp points");
    }
    else {
        if  (event == EVENT_LBUTTONDOWN){
            points.push_back(Vec2i( x, y ));
            if(points.size() >= 8) {
                warpedPointsReady = true;
                for(int i = 1; i < points.size(); i += 2){
                    Vec4i cLine = Vec4i(points[i-1][0], points[i-1][1], points[i][0], points[i][1]);
                    int ii = i == (points.size() - 1) ? 1 : (i+2);
                    Vec4i nLine = Vec4i(points[ii-1][0], points[ii-1][1], points[ii][0], points[ii][1]);
                    Vec2i intersection =  Geometry::intersect(cLine,nLine);
                    PERSPECTIVE_SRC_POINTS[(i-1)/2] = Point(intersection[0],intersection[1]);
                }
            }
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

    Image im = Image("2012-10-16_06_51_45.jpg");

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
            //line(plot2.getMat(), Point((*it)[0], (*it)[1]), Point((*it)[2], (*it)[3]), Scalar(0,0,255), 2, 8 );

        }else{
            verticalLines.push_back(*it);
            //line(plot2.getMat(), Point((*it)[0], (*it)[1]), Point((*it)[2], (*it)[3]), Scalar(0,255,0), 2, 8 );
            //circle( plot2.getMat(), Point( (*it)[0], (*it)[1] ), 5,  Scalar(255,255,0), 1, 5, 0 );
            //circle( plot2.getMat(), Point( (*it)[2], (*it)[3] ), 5,  Scalar(255,255,0), 1, 5, 0 );

        }
    }

    /*
    Filtered vectors will contain the new lines obtained from comparing lines that close to each other from horizontalLines and verticalLines
    auxiliarVec is a vector that will aid the following part of the algorithm
    current is the line currently being compared to others
    compared is the line currently being compared to current
    mean is the mean between values of the lines being compared
    */
    vector<Vec4i> horizontalLinesFiltered, verticalLinesFiltered, auxiliarVec;
    Vec4i current;
    Vec4i compared;
    int mean;

    /*
    This part of the algorithm picks a line from the vector horizontalLines and compares it to the others
    It approves a line is close to another if the distance in height between them is below a certain threshold (30 pixels in this case)
    */
    for (int i = 0; i < horizontalLines.size();i++) {
        current = horizontalLines[i];
        for (int j = i+1; j < horizontalLines.size();) {
            compared = horizontalLines[j];
            if (abs(current[1] - compared[1]) < 30) {
                auxiliarVec.push_back(compared);
                horizontalLines.erase(horizontalLines.begin() + j);
            }
            else {
                j++;
            }
        }

        //in order to try to eliminate certain false positives we eliminate lines who don't have at least 3 lines close to them
        if (auxiliarVec.size() > 2) {

            //auxIm is used to obtain the images number of columns (width)
            Mat auxIm = plot2.getMat();

            //drawing the line and adding it to the filtered vector
            line(plot2.getMat(), Point(0, current[1]), Point(auxIm.cols, current[3]), Scalar(255, 0, 255), 2, 8);
            horizontalLinesFiltered.push_back(Vec4i(0, current[1], auxIm.cols, current[3]));
        }

        //clear the auxiliar vector for the next round of lines being compared
        auxiliarVec.clear();
    }

    //clear the auxiliar vector for the next round of lines being compared
    auxiliarVec.clear();

    /*
    This part of the algorithm picks a line from the vector verticalLines and compares it to the others
    It approves a line is close to another if the distance in width between them is below a certain threshold (15 pixels in this case)
    and if the difference in heigh between their low/high points is below a certain threshold (60 pixels in this case)
    */
    for (int i = 0; i < verticalLines.size(); i++) {
        current = verticalLines[i];
        int highPoint = -1;
        int lowPoint = 999999;
        //search for the high/low point of the line
        if (current[1] > highPoint) highPoint = current[1];
        if (current[3] > highPoint) highPoint = current[3];
        if (current[1] < lowPoint) lowPoint = current[1];
        if (current[3] < lowPoint) lowPoint = current[3];

        //clear the auxiliar vector for the next round of lines being compared
        auxiliarVec.clear();

        for (int j = i + 1; j < verticalLines.size();) {
            compared = verticalLines[j];
            int comparedHighPoint = -1;
            int comparedLowPoint = 999999;
            //search for the high/low point of the line
            if (compared[1] > comparedHighPoint)
                comparedHighPoint = compared[1];
            if (compared[3] > comparedHighPoint)
                comparedHighPoint = compared[3];
            if (compared[1] < comparedLowPoint)
                comparedLowPoint = compared[1];
            if (compared[3] < comparedLowPoint)
                comparedLowPoint = compared[3];

            //check distances
            if (abs(current[0] - compared[0]) < 15 && (abs(lowPoint - comparedLowPoint) < 60 || abs(highPoint - comparedHighPoint) < 60)) {
                int stop = abs(current[0] - compared[0]);
                auxiliarVec.push_back(compared);
                verticalLines.erase(verticalLines.begin() + j);
            }
            else {
                j++;
            }
        }


        mean = current[0];
        //add to mean and check which of the lines is the tallest
        for (int k = 0; k < auxiliarVec.size(); k++) {
            Vec4i aux = auxiliarVec[k];
            if (aux[1] > highPoint)
                highPoint = aux[1];
            if (aux[3] > highPoint)
                highPoint = aux[3];
            if (aux[1] < lowPoint)
                lowPoint = aux[1];
            if (aux[3] < lowPoint)
                lowPoint = aux[3];

            mean += aux[0];
        }

        //final check on tallness
        if (current[1] > highPoint) highPoint = current[1];
        if (current[3] > highPoint) highPoint = current[3];
        if (current[1] < lowPoint) lowPoint = current[1];
        if (current[3] < lowPoint) lowPoint = current[3];

        //getting the x coordinate of the new line
        mean = (int)mean / (auxiliarVec.size() + 1);

        //drawing the line and adding it to the filtered vector
        line(plot2.getMat(), Point(mean, current[1]), Point(mean, current[3]), Scalar(0, 0, 0), 2, 8);
        verticalLinesFiltered.push_back(Vec4i(mean, lowPoint, mean, highPoint));
    }


    /*
     *  Calculating intersections between  vertical and horizontal edges
     */
    vector<Vec2i> intersectPoints;
    for(vector<Vec4i>::iterator it = verticalLinesFiltered.begin(); it != verticalLinesFiltered.end(); it++) {
        for (vector<Vec4i>::iterator itt = horizontalLinesFiltered.begin(); itt != horizontalLinesFiltered.end(); itt++) {
            Vec2i i =  Geometry::intersect(*it,*itt);
            int xi = i[0], x1 = min((*it)[0],(*it)[2]), x2 = max((*it)[0],(*it)[2]), x3 = min((*itt)[0],(*itt)[2]), x4 = max((*itt)[0],(*itt)[2]);
            int yi = i[1], y1 = min((*it)[1],(*it)[3]), y2 = max((*it)[1],(*it)[3]), y3 = min((*itt)[1],(*itt)[3]), y4 = max((*itt)[1],(*itt)[3]);

            if(xi >= x1 && xi <= x2 && xi >= x3 && xi <= x4 && yi >= y1 && yi <= y2 && yi >= y3 && yi <= y4){
                //circle( plot2.getMat(), Point( i[0], i[1] ), 5,  Scalar(255,0,0), 1, 5, 0 );
                intersectPoints.push_back(Vec2i(i[0], i[1]));

            }
        }
    }


    /*
     * Filtering / clustering intersection points
     */
    vector<Vec2i> intersectPointsFiltered, auxiliarVecPoint;
    for (int i = 0; i < intersectPoints.size(); i++) {
        Vec2i current = intersectPoints[i];

        auxiliarVecPoint.clear();
        for (int j = i + 1; j < intersectPoints.size();) {
            Vec2i compared = intersectPoints[j];
            if (abs(current[0] - compared[0]) < 20 && abs(current[1] - compared[1]) < 5) {
                auxiliarVecPoint.push_back(compared);
                intersectPoints.erase(intersectPoints.begin() + j);
            }
            else {
                j++;
            }
        }

        mean = current[0];


        for (int k = 0; k < auxiliarVecPoint.size(); k++) {
            Vec2i aux = auxiliarVecPoint[k];
            mean += aux[0];
        }

        mean = (int)mean / (auxiliarVecPoint.size() + 1);
        circle(plot2.getMat(), Point(mean, current[1]), 5, Scalar(255, 0, 0), 1, 5, 0);
        intersectPointsFiltered.push_back(Vec2i(mean, current[1]));
    }

    /*
     * attempt to retreive parking lots based on parallel lines
    std::pair<Vec4i, std::vector<Vec4i>> pairedVerticalLines;
    for(vector<Vec4i>::iterator it = verticalLinesFiltered.begin(); it != verticalLinesFiltered.end(); it++) {
        int y1 = max((*it)[1], (*it)[3]), y2 = min((*it)[1], (*it)[3]);
        vector<Vec4i> parallel;
        for(vector<Vec4i>::iterator itt = verticalLinesFiltered.begin(); itt != verticalLinesFiltered.end(); itt++) {
            //not the same
            if((*it)[0] == (*itt)[0] && (*it)[1] == (*itt)[1] && (*it)[2] == (*itt)[2] && (*it)[3] == (*itt)[3]) continue;

            //at least partially parallel
            if( ((*itt)[1] <= y1 && (*itt)[1] >= y2)  || ((*itt)[3] <= y1 && (*itt)[3] >= y2) ){
                parallel.push_back(*itt);
            }
        }

        Vec4i closest1st, closest2nd;
        int dst1st = 99999, dst2nd = 99999;
        //find the 2 closest edges
        for(vector<Vec4i>::iterator itt = verticalLinesFiltered.begin(); itt != verticalLinesFiltered.end(); itt++) {
            float dst = Geometry::distance(Geometry::midpoint(*it), Geometry::midpoint(*itt));
            if(dst < dst1st){
                dst2nd = dst1st;
                dst1st = dst;
                closest2nd = closest1st;
                closest1st = *itt;
            } else if(dst < dst2nd) {
                dst2nd = dst;
                closest2nd = *itt;
            }
        }

    }*/


    /*
     * Retrieval of parking spaces (ROIs):
     * 2
     */
    vector<Rect> lotsRoi;

    int lessDistance;
    int neighbourIndex;
    int edgeDistance;
    int edgeDistanceHigh;
    int edgeDistanceLow;
    Vec2i currentsEdge;
    Vec2i aux;
    Mat hey;
    hey = plot2.clone().getMat();
    int direction = -1; //Down -> 0, Up -> 1 , Undefined(ingore) -> -1
    bool ticked = false;
    // (
    for (int i = 0; i < intersectPointsFiltered.size(); i++) {
        Vec2i current = intersectPointsFiltered[i];
        lessDistance = 999999;
        // searching neighbour points
        for (int j = i + 1; j < intersectPointsFiltered.size();j++) {
            Vec2i compared = intersectPointsFiltered[j];
            int wid = abs(compared[0] - current[0]);
            if (abs(compared[0] - current[0]) < 60 && abs(compared[0] - current[0]) < lessDistance && abs(compared[1]-current[1]) < 25) {
                neighbourIndex = j;
                lessDistance = abs(compared[0] - current[0]);
                aux = compared;
                ticked = true;
            }
        }

        // searching vertical lines intersecting the neighbour point and determination of the orientation of the parking space
        for (int k = 0; k < verticalLinesFiltered.size(); k++) {
            Vec4i compared = verticalLinesFiltered[k];
            int high = max(compared[1], compared[3]);
            int low = min(compared[1], compared[3]);
            if (abs(compared[0] - current[0]) < 15 && current[1] >= low && current[1] <= high && (abs(compared[1] - low)< 30 || abs(compared[1] - high) < 30)) {

                edgeDistanceHigh = abs(current[1] - high);
                edgeDistanceLow = abs(current[1] - low);
                if (edgeDistanceLow < edgeDistanceHigh) {
                    if (edgeDistanceLow > 30 && edgeDistanceLow < 120) {
                        currentsEdge = Vec2i(current[0], min(compared[1], compared[3]));
                        direction = 1;
                    }
                    else if (edgeDistanceHigh > 30 && edgeDistanceHigh < 120) {
                        currentsEdge = Vec2i(current[0], max(compared[1], compared[3]));
                        direction = 0;
                    } else{
                        direction = -1;
                    }
                }
                else {
                    if (edgeDistanceHigh > 30 && edgeDistanceHigh < 120) {
                        currentsEdge = Vec2i(current[0], max(compared[1], compared[3]));
                        direction = 0;
                    }
                    else if (edgeDistanceLow > 30 && edgeDistanceLow < 120) {
                        currentsEdge = Vec2i(current[0], min(compared[1], compared[3]));
                        direction = 1;
                    } else{
                        direction = -1;
                    }
                }
            }
        }

        if (!ticked) {
            direction = -1;
        }

        ticked = false;

        // drawing and storing of ROI/parking spaces
        if (direction == 0) {
            int width = abs(current[0] - aux[0]);
            Rect rectan = Rect(current[0], current[1], abs(current[0] - aux[0]), abs(current[1] - currentsEdge[1]));
            int r1 = rand() % 255 + 0;
            int r2 = rand() % 255 + 0;
            int r3 = rand() % 255 + 0;
            cv::rectangle(hey, rectan, Scalar(r1, r2, r3), 2);
            circle(hey, Point(current[0], current[1]), 5, Scalar(r1, r2, r3), 3, 2);
            lotsRoi.push_back(rectan);
        }
        else if(direction == 1){
            int width = abs(current[0] - aux[0]);
            Rect rectan = Rect(currentsEdge[0], currentsEdge[1], abs(current[0] - aux[0]), abs(current[1] - currentsEdge[1]));
            int r1 = rand() % 255 + 0;
            int r2 = rand() % 255 + 0;
            int r3 = rand() % 255 + 0;
            cv::rectangle(hey, rectan, Scalar(r1, r2, r3), 2);
            circle(plot2.getMat(), Point(currentsEdge[0], currentsEdge[1]), 5, Scalar(r1, r2, r3), 3, 2);
            lotsRoi.push_back(rectan);
        }

    }


    //warping perspective of the image to test accordingly to the test image points
    im.wrapPerspective(PERSPECTIVE_SRC_POINTS, PERSPECTIVE_DST_POINTS).gaussianBlur(9);


    // Tuning the 'test image' colour in order to become similar to the reference one using the difference of the average between the two images
    Mat emptyMat = Image(hey).getMat();
    Mat imMat = im.getMat().clone();

    Scalar meanEmpty = cv::mean(emptyMat);
    Scalar meanIm = cv::mean(imMat);
    Scalar diff = meanEmpty - meanIm;

    imMat  = imMat + diff;



    // comparing the ROIs/parking spaces in the empty parking lot image against the test image
    for(vector<Rect>::iterator it = lotsRoi.begin(); it != lotsRoi.end(); it++) {
        if((*it & cv::Rect(0, 0, emptyMat.cols, emptyMat.rows)) == (*it)){
            Mat segEmptyMat =  emptyMat(*it);
            Mat segImMat = imMat(*it);
            Mat segAbsDiff;
            absdiff(segEmptyMat, segImMat, segAbsDiff);

            Scalar diffMeans = sum(segAbsDiff);
            int totalSum = abs(diffMeans[0]) + abs(diffMeans[1]) + abs(diffMeans[2]);

            if(totalSum > 150000){
                cv::rectangle(imMat, *it, Scalar(0, 200, 0), 2); // draws green occupied parking space
            }else{
                cv::rectangle(imMat, *it, Scalar(0, 0, 200), 2); // draws red free parking space
            }
        }

    }

    Image(emptyMat).show("Empty", 0);
    Image(imMat).show("Cars",0);

    waitKey(0);
    return 0;

}