#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <uchar.h>
#include <math.h>
#include <iostream>
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <fstream> 
using namespace cv;
using namespace std;
int pixel[200][200], fd2[200][200], fd3[200][200], fd4[200][200], fd1[200][200], ldgp[200][200], hist[49][64],rowRegion,colRegion;
void show_error(unsigned int handletype, const SQLHANDLE& handle) {
	SQLWCHAR sqlstate[1024];
	SQLWCHAR message[1024];
	if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
		cout << "Message: " << message << "nSQLSTATE: " << sqlstate << endl;
}
void copyPixel(int fd[][200], int row, int col)//Updating values of the global array fd 
{
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			fd[i][j] = pixel[i][j];
		}
	}
}

void compute1(int row, int col)//Iterative formula for pixels at 0 degree
{
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			pixel[i][j] = fd1[i][j] - fd1[i][j + 1];
		}
	}
	copyPixel(fd1, row, col);
}

void compute2(int row, int col)//Iterative formula for pixels at 45 degrees
{
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			pixel[i][j] = fd1[i][j] - fd1[i][j + 1];
		}
	}
	copyPixel(fd1, row, col);
}

void compute3(int row, int col)//Iterative formula for pixels at 90 degrees
{
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			pixel[i][j] = fd3[i][j] - fd3[i - 1][j];
		}
	}

	copyPixel(fd3, row, col);
}

void compute4(int row, int col)//Iterative formula for pixels at 135 degrees
{
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			pixel[i][j] = fd4[i][j] - fd4[i - 1][j - 1];
		}
	}

	copyPixel(fd4, row, col);
}
void ldgpGen(int fd1[][200], int fd2[][200], int fd3[][200], int fd4[][200], int row, int col)//To compute the nth order ldgp values 
{
	int sum = 0, a[6] = { 0 };
	for (int i = 10; i < row + 10; i++)
	{
		for (int j = 10; j < col + 10; j++)
		{
			if (fd1[i][j] > fd2[i][j])
				a[5] = 1;
			if (fd1[i][j] > fd3[i][j])
				a[4] = 1;
			if (fd1[i][j] > fd4[i][j])
				a[3] = 1;
			if (fd2[i][j] > fd3[i][j])
				a[2] = 1;
			if (fd2[i][j] > fd4[i][j])
				a[1] = 1;
			if (fd3[i][j] > fd4[i][j])
				a[0] = 1;
			for (int k = 0; k < 6; k++)
			{
				sum = sum + a[k] * (pow(2 ,k));
			}
			ldgp[i][j] = sum;
			sum = 0;
			for (int k = 0; k < 6; k++)
			{
				a[k]= 0;
			}
		}
	}
}
void spatialHist(int ldgp[][200])//Calculating the spatial histogram
{
	for (int i = 0;i < 49;i++)
	{
		for (int j = 0;j < 64;j++)
		{
			hist[i][j] = { 0 };
		}
	}
	int part = 0;
	for (int m = 0;m < 6;m++)
	{
		for (int i = m*rowRegion;i < ((m +1) *rowRegion);i++)
		{
			for (int n = 0;n < 6;n++)
			{
				for (int j = n*colRegion;j < (n+1)*colRegion;j++)
				{
					for (int k = 0;k < 64;k++)
					{
						if (k == ldgp[i][j])
							hist[part][k]++;
					}
				}
			}
			part++;
		}
	}
	for (int i = 0;i < 49;i++)
	{
		for (int j = 0;j < 64;j++)
		{
			//printf("%d ", hist[i][j]);
		}
		// printf("\n");
	}
}
void checkHist()//Function to check the distances between query image and gallery images
{
	int summation=0;
	printf("\n\n");
	int check[49][64];
	ifstream ifile("ldgpDB2.txt",ios::binary);
	if (!ifile)
	{
		printf("\nFile not opened for reading");
	}
	int k = 0;
	while (!ifile.eof())
	{
		for (int i = 0;i < 49;i++)//Check matrices of 49*64 to find the distance matrices (taxicab distance)
		{
			for (int j = 0;j < 64;j++)
			{
				ifile.read((char*)&k, sizeof(k));//Read ldgp values of stored images from the database file
				check[i][j] = abs(hist[i][j] - k);
				summation = summation + check[i][j];//Find sum of all the distances
				printf("%d ", check[i][j]);
			}
			printf("\n");
		}
		printf("\nSum of all the distance values:%d", summation);
		summation = 0;
		printf("\n\n");
	}
	ifile.close();
}
int main(int argc, char** argv)
{
	Mat src, src1;
	Mat grad;
	int c = 0, row, col, n;
	char* window_name = "MyWindow";
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	src1 = imread("C:\\Users\\CHHANDAK BAGCHI\\Documents\\Visual Studio 2015\\Projects\\ImageExtract1\\data\\yaleB01_P00A+000E+00.pgm", CV_LOAD_IMAGE_GRAYSCALE); //Reading the query image
	row = src1.rows;
	col = src1.cols;
	rowRegion = (src1.rows) / 7;
	colRegion = (src1.cols) / 7;
	imwrite("C:\\Users\\CHHANDAK BAGCHI\\Documents\\Visual Studio 2015\\Projects\\ImageExtract1\\data\\yaleB01_P00A+000E+00.pgm.jpg", src1); //Writing it to a jpeg file
	copyMakeBorder(src1, src, 10, 10, 10, 10, BORDER_CONSTANT, 0);
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			pixel[i][j] = src.at<uchar>(i, j);// Reading intensity values of each pixel
		}
	}
	printf("\nEnter the value for n: ");
	scanf("%d", &n);//Reading the value of n to compute nth order ldgp
	for (int i = 10; i < row + 10; i++)//Finding ldgp of order 1
	{
		for (int j = 10; j < col + 10; j++)
		{
			fd1[i][j] = pixel[i][j] - pixel[i][j + 1];
			fd2[i][j] = pixel[i][j] - pixel[i - 1][j + 1];
			fd3[i][j] = pixel[i][j] - pixel[i - 1][j];
			fd4[i][j] = pixel[i][j] - pixel[i - 1][j - 1];
		}
	}

	ldgpGen(fd1, fd2, fd3, fd4, row, col);
	for (int k = 2; k < n - 2; k++)//Iterative code to find ldgp of orders 2,3...n
	{
		compute1(row, col);
		compute2(row, col);
		compute3(row, col);
		compute4(row, col);

		ldgpGen(fd1, fd2, fd3, fd4, row, col);//Function called to create the ldgp matrix
	}
	spatialHist(ldgp);//Function called to create spatial histogram
	checkHist();//Function called to check whether image exists in the database
	cv::Mat ldgn(row, col, CV_16S, ldgp);
	imwrite("C:\\Users\\CHHANDAK BAGCHI\\Documents\\Visual Studio 2015\\Projects\\ImageExtract1\\data\\ldgn.jpg", ldgn);//Writing the nth order ldgp image
	copyMakeBorder(ldgn, ldgn, 10, 10, 10, 10, BORDER_CONSTANT, 0);
	system("pause");
}