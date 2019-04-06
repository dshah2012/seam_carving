#include "sc.h"

using namespace cv;
using namespace std;
#include<cmath>
#include<stack>




bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }
    
    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;

    }
    
    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;
        
    } 
    return seam_carving_trivial(in_image, new_width, new_height, out_image);
}

void clockWiseRotation(Mat& in_image, Mat& out_image) {
    rotate(in_image, out_image, ROTATE_90_CLOCKWISE);
    out_image.copyTo(in_image);
	cout<<"Rotated"<<endl;
}

void antiClockWiseRotation(Mat& in_image, Mat& out_image) {
    rotate(in_image, out_image, ROTATE_90_COUNTERCLOCKWISE);
    out_image.copyTo(in_image);
}

bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image){

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
 while(iimage.cols!=new_width){
        if(iimage.cols>new_width){
			cout<<iimage.cols<<endl;
            reduce_vertical_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
 }
 clockWiseRotation(iimage, oimage);
 while(iimage.cols!=new_height){
		if(iimage.cols>new_height){
			cout<<iimage.cols<<endl;
            reduce_vertical_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }
    }
 antiClockWiseRotation(iimage, oimage);
	//rotate_image_anticlockwise(iimage, oimage);
 out_image = oimage.clone();
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
    out_image = Mat(rows, cols-1, CV_8UC3);
    Mat energy=getEnergy(in_image);
	int finalEnergyvalues[rows][cols];
	for(int i=cols-1;i>0;--i)     
	{
             finalEnergyvalues[0][i]=(int)energy.at<uchar>(0,i);      
    }
	
	
	for(int r = 1; r < rows; r++){
        for(int c = 0; c < cols; c++){
            if (c == 0)
                finalEnergyvalues[r][c] = min(finalEnergyvalues[r-1][c+1], finalEnergyvalues[r-1][c]);
            else if (c == cols-1)
                finalEnergyvalues[r][c] = min(finalEnergyvalues[r-1][c-1], finalEnergyvalues[r-1][c]);
            else
                finalEnergyvalues[r][c] = min({finalEnergyvalues[r-1][c-1], finalEnergyvalues[r-1][c], finalEnergyvalues[r-1][c+1]});
            finalEnergyvalues[r][c] += (int)energy.at<uchar>(r,c);
        }
    }
    
   


	int min_value,min_value2;
	min_value=numeric_limits<int>::max();
	int min_index=0;

//finding meanimum in topmost row of image

	for(int i=0;i<cols;++i)
      {
            if(min_value>finalEnergyvalues[rows-1][i])
            {
				min_value=finalEnergyvalues[rows-1][i];
				min_index=i;
            }
      }

	stack<int> s;
	s.push(min_index);


	for(int i=rows-1;i>0;--i)
      {
            if(min_index==0)
                  {
					  min_value2=min(finalEnergyvalues[i-1][min_index],finalEnergyvalues[i-1][min_index+1]);
						if(min_value2==finalEnergyvalues[i-1][min_index+1])
                        {
							min_index=min_index+1;
                        }
                       s.push(min_index);
                     }
             else if(min_index==cols-1)
                   {
					   min_value2=min(finalEnergyvalues[i-1][min_index],finalEnergyvalues[i-1][min_index-1]);
					   if(min_value2==finalEnergyvalues[i-1][min_index-1])
					   {
						  min_index=min_index-1;
					   }
					   s.push(min_index);
                    }
			  else
               {
                     min_value2=min(finalEnergyvalues[i-1][min_index],min(finalEnergyvalues[i-1][min_index-1],finalEnergyvalues[i-1][min_index+1]));
                      if(min_value2==finalEnergyvalues[i-1][min_index-1])
                         {
							 min_index=min_index-1;
                         }
                       else if(min_value2==finalEnergyvalues[i-1][min_index+1])
                       {
						   min_index=min_index+1;
                       }
                       s.push(min_index);
               }

	}
	for (int i = 0; i< rows; ++i)
	{
		if (s.top()<cols-1)
		{
			for (int j = 0;j<s.top();++j)
			{
                Vec3b pixel = in_image.at<Vec3b>(i, j);
				out_image.at<Vec3b>(i,j) = pixel;		
		    }        
			for (int j = s.top()+1;j<cols;++j) 
            {
				Vec3b pixel = in_image.at<Vec3b>(i,j);	
                out_image.at<Vec3b>(i,j-1)=pixel;	
		    }
	     }
		else 
		{
			for (int j = 0;j<cols-1;++j)
            {
				Vec3b pixel = in_image.at<Vec3b>(i,j);	
                out_image.at<Vec3b>(i,j)=pixel;	
		    }
	     }
		s.pop();
	}
    return true;
}