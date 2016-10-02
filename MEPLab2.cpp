//Michael Piechota (889378587)
//3/29/16
//CS 546 Lab Assignment 2
//Write a program based on the Week 5 Lab Exercise program B to read in the file of destinations and plot them on a bit map.
//The dimensions should be 1024 by 1024 pixels. The background color should be black. The center point of the plot area should be latitude 40.765482, longitude -73.980006. 
//(This the corner of 7th Avenue and West 57th Street, i.e. Carnegie Hall.) The southern boundary should be roughly 40.700455 and the northern boundary should be roughly 40.830509. 
//The eastern and western boundaries will be proportionate to these.
#include <iostream>
#include <fstream>
#include "windows.h"
#define IMAGE_SIZE 1024
using namespace std;

struct coordinates
{
	int x;
	int y;
	bool valid;
};

const float centerLatitude = 40.765482f;//center latitude point
const float centerLong = -73.980006f;//center longitude point
const float mapSize = 40.830509f - 40.700455f;//LatLong size of the map
const float startingLatitude = centerLatitude - mapSize / 2;
const float startLong = centerLong - mapSize / 2;
const byte scale = 1;
coordinates setPair(char * bytes);

int main()
{
	char table[1024];
	static char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };
	ifstream file("L2Data10K.dat", ios::binary);
	int pairs = 0;
	int failedPoints = 0;
	int overSatPts = 0;
	char coordinatePairBytes[8];
	coordinates * coordinatePairs;

	if (file.is_open())
	{
		file.seekg(0, file.end);
		int fileLength = (int)file.tellg();
		if (fileLength % 8 != 0)
		{
			cout << "*Error* The length of the file is INVALID" << endl;
			return -1;
		}
		
		file.seekg(0, ios::beg);
		pairs = fileLength / 8;
		
		coordinatePairs = new coordinates[pairs];
		for (int i = 0; i < pairs; i++)
		{
			file.read(coordinatePairBytes, 8);
			coordinatePairs[i] = setPair(coordinatePairBytes);
		}
		file.close();
	}
	else
	{
		cout << "ERROR OPENING FILE";
		return -1;
	}

	ofstream bmpOut("MEP_Lab2_output.bmp", ios::out + ios::binary);
	if (!bmpOut) {
		cout << "ERROR OPENING IMAGE";
		return -1;
	}

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	bmfh.bfType = 0x4d42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(table);
	bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);
	bmih.biSize = 40;
	bmih.biWidth = IMAGE_SIZE;
	bmih.biHeight = IMAGE_SIZE;
	bmih.biPlanes = 1;
	bmih.biBitCount = 8;
	bmih.biCompression = 0;
	bmih.biSizeImage = IMAGE_SIZE * IMAGE_SIZE;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;

	byte color = 0;
	for (int i = 0; i <= scale; i++) {
		int j = i * 4;
		if (i > 0)
		{
			color = i * (255 / scale) + 255 % scale;
		}
		table[j] = table[j + 1] = table[j + 2] = table[j + 3] = color;
	}

	for (int i = 0; i < pairs; i++)
	{
		if (coordinatePairs[i].valid )
		{
			if (bits[coordinatePairs[i].x][coordinatePairs[i].y] < scale)
			{
				bits[coordinatePairs[i].x][coordinatePairs[i].y]++;
			}
			else
			{
				overSatPts++;
			}
		}
		else
		{
			failedPoints++;
		}
	}
	delete[] coordinatePairs;
	char* workPtr;
	workPtr = (char*)&bmfh;
	bmpOut.write(workPtr, 14);
	workPtr = (char*)&bmih;
	bmpOut.write(workPtr, 40);
	workPtr = &table[0];
	bmpOut.write(workPtr, sizeof(table));
	workPtr = &bits[0][0];
	bmpOut.write(workPtr, IMAGE_SIZE*IMAGE_SIZE);
	bmpOut.close();

	system("pause");
	system("mspaint MEP_Lab2_output.bmp");
	return 0;
}

coordinates setPair(char * bytes)
{
	char xBytes[] = { bytes[0], bytes[1], bytes[2], bytes[3] };
	char yBytes[] = { bytes[4], bytes[5], bytes[6], bytes[7] };
	coordinates latitudeLong;
	float imageSize = IMAGE_SIZE;
	float xTemp = 0;
	float yTemp = 0;
	short cWord = 0;
	__asm
	{
		PUSH	EAX
		MOV		EAX, 0
		MOV		AH, xBytes[3]
		MOV		AL, xBytes[2]
		SHL		EAX, 16
		MOV		AH, xBytes[1]
		MOV		AL, xBytes[0]
		MOV		xTemp, EAX
		MOV		EAX, 0
		MOV		AH, yBytes[3]
		MOV		AL, yBytes[2]
		SHL		EAX, 16
		MOV		AH, yBytes[1]
		MOV		AL, yBytes[0]
		MOV		yTemp, EAX
		POP		EAX
		FINIT
		FSTCW	cWord
		MOV	AX, cWord
		AND	AX, 0xFCFF
		MOV	cWord, AX
		FLDCW	cWord
		FLD		xTemp
		FSUB	startingLatitude
		FMUL	imageSize
		FDIV	mapSize
		FISTP	latitudeLong.x
		FLD		yTemp
		FSUB	startLong
		FMUL	imageSize
		FDIV	mapSize
		FISTP	latitudeLong.y
	}

	latitudeLong.valid = latitudeLong.x 
	<= IMAGE_SIZE && latitudeLong.x >= 0 && latitudeLong.y 
	<= IMAGE_SIZE && latitudeLong.y >= 0;
	return latitudeLong;
}