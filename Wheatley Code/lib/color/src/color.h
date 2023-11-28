#ifndef COLOR_H  
#define COLOR_H
#include <math.h>
struct RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct HSV
{
	double H;
	double S;
	double V;
};
struct RGB HSVToRGB(struct HSV hsv);
#endif