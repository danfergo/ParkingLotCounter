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