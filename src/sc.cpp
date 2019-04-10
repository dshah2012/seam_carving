
#include "sc.h"

using namespace cv;
using namespace std;

void deleteunWantedSeam(Mat &,vector<int>);

bool seam_carving(Mat &in_image, int new_width, int new_height, Mat &out_image) {

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if (new_width > in_image.cols) {
        cout << "Invalid request!!! new_width has to be smaller than the current size!" << endl;
        return false;
    }
    if (new_height > in_image.rows) {
        cout << "Invalid request!!! ne_height has to be smaller than the current size!" << endl;
        return false;
    }

    if (new_width <= 0) {
        cout << "Invalid request!!! new_width has to be positive!" << endl;
        return false;

    }

    if (new_height <= 0) {
        cout << "Invalid request!!! new_height has to be positive!" << endl;
        return false;

    }


    return seam_carving_trivial(in_image, new_width, new_height, out_image);
}


void clockWiseRotation(Mat& in_image, Mat& out_image) {
    rotate(in_image, out_image, ROTATE_90_CLOCKWISE);
    out_image.copyTo(in_image);
}

void antiClockWiseRotation(Mat& in_image, Mat& out_image) {
    rotate(in_image, out_image, ROTATE_90_COUNTERCLOCKWISE);
    out_image.copyTo(in_image);
}


bool seam_carving_trivial(Mat &in_image, int new_width, int new_height, Mat &out_image) {

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
 while(iimage.cols!=new_width){
        if(iimage.cols>new_width){
		//cout<<iimage.cols<<endl;
           //	cout<<"In rotation";
		reduce_vertical_seam_trivial(iimage, oimage);
            //iimage = oimage.clone();
        }
 }
 clockWiseRotation(iimage, oimage);
 while(iimage.cols!=new_height){
		if(iimage.cols>new_height){
			//cout<<iimage.cols<<endl;
            reduce_vertical_seam_trivial(iimage, oimage);
            //iimage = oimage.clone();
        }
    }
 antiClockWiseRotation(iimage, oimage);
	//rotate_image_anticlockwise(iimage, oimage);
 out_image = iimage.clone();
 return true;
}

Mat getEnergy(Mat &in_image){
	
	
	Mat src, src_gray;
  	Mat energy;
	int scale = 1;
  	int delta = 0;
    int ddepth = CV_16S;
	src = in_image.clone();
   	GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
    cvtColor( src, src_gray, CV_BGR2GRAY );
	Mat grad_x, grad_y;
  	Mat abs_grad_x, abs_grad_y;
    Sobel( src_gray, grad_x, ddepth, 1, 0,3, scale, delta, BORDER_DEFAULT );
    Sobel( src_gray, grad_y, ddepth, 0, 1,3,scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_x, abs_grad_x );
	convertScaleAbs( grad_y, abs_grad_y );
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, energy);
	return energy;
}


bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image){
	int rows = in_image.rows;
    int cols = in_image.cols;
Mat energy=getEnergy(in_image);

	int finalEnergyvalues[rows][cols];
	for(int i=cols-1;i>0;--i)     
	{
             finalEnergyvalues[0][i]=(int)energy.at<char>(0,i);      
    	}
	
	for(int i=1;i<rows;++i){
        	for(int j=0;j<cols;++j){
        	    if(j==0)
        	    {
			 finalEnergyvalues[i][j]=min(abs(energy.at<char>(i,j)-energy.at<char>(i-1,j))+finalEnergyvalues[i-1][j],abs(energy.at<char>(i,j)-energy.at<char>(i-1,j+1))+finalEnergyvalues[i-1][j+1]);
               	    }
              	   else if(j==cols-1)
                   {
			finalEnergyvalues[i][j]=min(abs(energy.at<char>(i,j)-energy.at<char>(i-1,j))+finalEnergyvalues[i-1][j],abs(energy.at<char>(i,j)-energy.at<char>(i-1,j-1))+finalEnergyvalues[i-1][j-1]);
		   }
             	   else
               	   {
			finalEnergyvalues[i][j]=min(abs(energy.at<char>(i,j)-energy.at<char>(i-1,j))+finalEnergyvalues[i-1][j],min(abs(energy.at<char>(i,j)-energy.at<char>(i-1,j-1))+finalEnergyvalues[i-1][j-1],abs(energy.at<char>(i,j)-energy.at<char>(i-1,j+1))+finalEnergyvalues[i-1][j+1]));
	           }
	}
    }

int min_value;
	min_value=numeric_limits<int>::max();
	int min_index=0;

	for(int i=0;i<cols;++i)
      {
            if(min_value>finalEnergyvalues[rows-1][i])
            {
				min_value=finalEnergyvalues[rows-1][i];
				min_index=i;
            }
      }
    vector<int> path(rows);
    Point position(rows - 1, min_index);
    path[position.x] = position.y;
	int value=0;
    while(position.x != 0) {
        int row = position.x, col = position.y;
        //int value = finalEnergyvalues[row][col] - (int)energy.at<uchar>(row,col);
        if(col == 0) {
			value=min(finalEnergyvalues[row-1][col],finalEnergyvalues[row-1][col+1]);
            if(value == finalEnergyvalues[row-1][col]) {
                position = Point(row-1, col);
            } else {
                position = Point(row-1, col+1);
            }
        } else if(col == cols - 1) {
	    value=min(finalEnergyvalues[row-1][col],finalEnergyvalues[row-1][col-1]);
            if(value == finalEnergyvalues[row-1][col]) {
                position = Point(row-1, col);
            } else {
                position = Point(row-1, col-1);
            }
        } else {
	   value=min(finalEnergyvalues[row-1][col],min(finalEnergyvalues[row-1][col-1],finalEnergyvalues[row-1][col+1]));
            if(value == finalEnergyvalues[row-1][col-1]) {
                position = Point(row-1, col-1);
            } else if(value == finalEnergyvalues[row-1][col]) {
                position = Point(row-1, col);
            } else {
                position = Point(row-1, col+1);
            }
        }
        path[position.x] = position.y;
	}
	deleteunWantedSeam(in_image,path);
	//in_image.copyTo(out_image);
	return true;
}

void deleteunWantedSeam(Mat &ou_image, vector<int> path) {
    int rows = ou_image.rows, cols = ou_image.cols;
    Mat inermediateImage = Mat(rows, cols - 1, CV_8UC3);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols-1; j++) {
            if(j >= path[i])
                inermediateImage.at<Vec3b>(i, j) = ou_image.at<Vec3b>(i, j+1);
            else
                inermediateImage.at<Vec3b>(i, j) = ou_image.at<Vec3b>(i, j);
        }
    }
    inermediateImage.copyTo(ou_image);
}

