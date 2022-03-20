#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#define width 250
#define height 360
using namespace std;

double pi = 3.14159;
double root2 = 1.41421;
int pic[height][width];

double COS[8][8], C[8];
float dct[360][248], idct[360][248], newdct[360][248];

vector <int> stored;
int zig[64];
int arr[64];




void readIMG(string fileName){

	fstream in;
	in.open(fileName.c_str());

	string temp;

	for(int q = 0; q < 4; q++){
		getline(in, temp);
	}

	int input;
	int rowCount = 0;
	int colCount = 0;
	while(in >> input){
		pic[rowCount][colCount] = input;
		colCount ++;

		if (colCount >= width){
			colCount = 0;
			rowCount ++;
		}

	}
	
}


void init(){
	int v, d;
	for (v = 0; v < 8; v++){
		for (d = 0; d < 8; d++){
			COS[v][d] = cos((2 * v + 1) * d * pi / 16.0);
			if (v) 	
				C[v] = 1;
			else 
				C[v] = 1 / root2;
		
		}
	}
}

void dct_ize(){

	int r, c, i, j, x, y;
	double sum = 0;
	for (r = 0; r < 45; r++){
		for (c = 0; c < 31; c++){
			for (i = 0; i < 8; i++){
		        for (j = 0; j < 8; j++) {
		          sum = 0;
		          for (x = 0; x < 8; x++){
		            for (y = 0; y < 8; y++){
		            	sum += (pic[r * 8 + x][c * 8 + y] - 128) * COS[x][i] * COS[y][j];
		          	}
				  }
		          sum *= C[i] * C[j] * 0.25;
		          dct[r * 8 + i][c * 8 + j] = sum;
				}
			}
		}
	}
}



void quantize_variable(int quality = 50){
	int table[8][8] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 82,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
	};
	
	double compression_scaler;
	
	
	if (quality > 50){
		for (int k = 0; k < 8; k++){
			for (int m = 0; m < 8; m++){
				compression_scaler = ((100 - quality)) / (float)50;
				table[k][m] = round(table[k][m] * compression_scaler);
				if (table[k][m] > 255){
					table[k][m] = 255;
				} else if (table[k][m] < 1){
					table[k][m] = 1;
				}
				
			}
		}
	}else if (quality < 50){
		for (int k = 0; k < 8; k++){
			for (int m = 0; m < 8; m++){
				compression_scaler = float(50 / quality);
				table[k][m] = round(table[k][m] * compression_scaler);
				if (table[k][m] > 255){
					table[k][m] = 255;
				} else if (table[k][m] < 1){
					table[k][m] = 1;
				}
				
			}
		}
	}
	
	
	int r, c, i, j;
	for (r = 0; r < 45; r++){
    	for (c = 0; c < 31; c++){
    		for (i = 0; i < 8; i++){
        		for (j = 0; j < 8; j++) {
          			dct[r * 8 + i][c * 8 + j] = round(dct[r * 8 + i][c * 8 + j] / table[i][j]);
          			dct[r * 8 + i][c * 8 + j] = dct[r * 8 + i][c * 8 + j] * table[i][j];
        		}
    		}
		}
	}
	/*
	for (i = 0; i < 8; i++){
     	for (j = 0; j < 8; j++) {
          	cout << dct[i][j] << ", "; 
        }
        cout << endl;
    }
    */
}



void runLengthEncoding(){
	bool isUp;
	int r, c, i, j;
	

	
	for (r = 0; r < 45; r++){
		for (c = 0; c < 31; c++){
			isUp = true;
			i = 0, j = 0;
			for (int k = 0; k < 64; ){
				if (isUp) {
           			for (; i >= 0 && j < 8; j++, i--) {
                		zig[k] = dct[r * 8 + i][c * 8 + j];
                		k++;
           			}
		            if (i < 0 && j <= 8 - 1)
		                i = 0;
		            if (j == 8)
		                i = i + 2, j--;
		        }
	        	else {
	            	for (; j >= 0 && i < 8; i++, j--) {
	                	zig[k] = dct[r * 8 + i][c * 8 + j];
	                	k++;
	            	}
		            if (j < 0 && i <= 8 - 1)
		                j = 0;
		            if (i == 8)
		                j = j + 2, i--;
	        	}
        		isUp = !isUp;
        		
			}
			for (int m = 0; m < 64; m++){
				if(zig[m] != 0){
					stored.push_back(zig[m]);
				}else{
					int count = 1;
		            while (m < 63 && zig[i] == zig[m + 1]){
		                count ++;
		                m++;
		            }   
		            stored.push_back(zig[m]);
		            stored.push_back(count);
				}
			}		
		}
	}
}

void deRunLengthEncoding(){
	int r, c, i, j;
	int storedCounter = 0;
	bool isUp;
	for (r = 0; r < 45; r++){
		for (c = 0; c < 31; c++){
			for (i = 0; i < 64; i++){
				if (stored.at(storedCounter) != 0){
					arr[i] = stored.at(storedCounter);
					storedCounter ++;
				} else {
					for(int x = 0; x < stored.at(storedCounter + 1); x++){
						arr[i] = 0;
						i++;
					}
					i--;
					storedCounter += 2;
				}
			} // fill 8x8 into 64 1d array from RLE vector, use arr[64] for reverse zig zag back into idct

			isUp = true;
			i = 0, j = 0;
			for (int k = 0; k < 64; ){
		        if (isUp) {
		            for (; i >= 0 && j < 8; j++, i--) {
		                newdct[r * 8 + i][c * 8 + j] = arr[k];
		                k++;
		            }
		 
		            if (i < 0 && j <= 8 - 1)
		                i = 0;
		            if (j == 8)
		                i = i + 2, j--;
		        }
		 
		        else {
		            for (; j >= 0 && i < 8; i++, j--) {
		                newdct[r * 8 + i][c * 8 + j] = arr[k];
		                k++;
		            }
		 
		            if (j < 0 && i <= 8 - 1)
		                j = 0;
		            if (i == 8)
		                j = j + 2, i--;
		        }
		 
		        isUp = !isUp;
			}
		}
	}
	
	
}









void idct_ize(int fileChoose){
	int r, c, i, j, x, y;
	for (r = 0; r < 45; r++){
		for (c = 0; c < 31; c++){
			for (i = 0; i < 8; i++){
		        for (j = 0; j < 8; j++) {
		          	double sum = 0;
		          	for (x = 0; x < 8; x++){
		            	for (y = 0; y < 8; y++){
		            	  	sum += C[x] * C[y] * newdct[r * 8 + x][c * 8 + y] * COS[i][x] * COS[j][y];
		            	}
					}
		        sum *= 0.25;
		        sum += 128;
		        idct[r * 8 + i][c * 8 + j] = sum;
		        
		        if (idct[r * 8 + i][c * 8 + j] > 255){
		        	idct[r * 8 + i][c * 8 + j] = 255;
				} else if(idct[r * 8 + i][c * 8 + j] < 0){
					idct[r * 8 + i][c * 8 + j] = 0;
				}
				
		    	}
			}
		}
	}
	
	string filePaths[10] = {
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct10.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct20.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct30.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct40.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct50.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct60.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct70.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct80.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct90.txt",
	"D:\\Misc\\DCT\\new_idct_txt_files\\idct100.txt"
};

	ofstream myfile;
	
	myfile.open(filePaths[fileChoose].c_str(), ios::app);
	for (int h = 0; h < 360; h++){
		for (int z = 0; z < 248; z++){
			myfile << round(idct[h][z]) << " ";
		}
		myfile << endl;
	}
	myfile.close();
	
	
	
}


void MSE(int quality = 50){
	double MSE = 0;
	int r, c;
	for (r = 0; r < 360; r++){
    	for (c = 0; c < 248; c++) {
			MSE += (pic[r][c] - idct[r][c]) * (pic[r][c] - idct[r][c]);
		}
	}
	MSE /= (360 * 428);
	double PSNR = 10 * log10(255 * 255 / MSE);
	cout << "MSE(" << quality << "): " << MSE << endl;
	cout << "PSNR: " << PSNR << endl;
}


void minmax(){
	int max = 0;
	int min = 10;
	int x, y;
	int a, b;
	for (int i = 0; i < 360; i++){
		for (int j = 0; j < 248; j++){
			if (idct[i][j] > max){
				max = idct[i][j];
				x = i;
				y = j;
			}
			if (idct[i][j] < min){
				a = i;
				b = j;
				min = idct[i][j];
			}
		}
	}
	cout << endl << "max: " << max << endl;
	cout << x << " " << y << endl;
	cout << endl << "min: " << min << endl;
	cout << a << " " << b;	
} // not really used




int main() {

	readIMG("D:\\Misc\\DCT\\mona_lisa.ascii.pgm");
	init();

	for (int i = 1; i <= 10; i++){
		dct_ize();
		quantize_variable(i * 10); // 50 is standard, range 1-100
		quantize_variable(50);
		runLengthEncoding();
		idct_ize(i-1);
		//MSE(i * 10);
		cout << i << "   " << (stored.size() * 14) << " " << (348 * 360 * 8) << "      " << ((348 * 360 * 8) - (stored.size() * 14)) << endl;
		stored.clear();
	}
	
	return 0;
}



















