#include "lutgenerator.h"
#include <QDebug>
#include <math.h>
#include <QtCore/qmath.h>
using namespace std;

#pragma pack(2)
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long    LONG;
typedef struct {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER;

// @brief: create LUT and show LUT
// @param width_: The size of the LUT
// @param intensity_: The intensity of integration
// @param radius_: The radius of integration
// @param color_: The color space of the LUT
// @param toneMap_: Use tonemap or not
// @ret: void
LutGenerator::LutGenerator(int width_, float intensity_, float radius_, bool color_, bool toneMap_)
{
    width = width_;
    intensity = intensity_;
    radius = radius_;
    color = color_;
    toneMapping = toneMap_;
    draw();
    qDebug()<<"finish drawing"<<endl;
}

// @brief: create LUT and save LUT
// @param width_: The size of the LUT
// @param intensity_: The intensity of integration
// @param radius_: The radius of integration
// @param color_: The color space of the LUT
// @param toneMap_: Use tonemap or not
// @param filename_: The saving address
// @ret: void
LutGenerator::LutGenerator(int width_, float intensity_, float radius_, bool color_, bool toneMap_, std::string filename_)
{
    width = width_;
    intensity = intensity_;
    radius = radius_;
    color = color_;
    toneMapping = toneMap_;
    save(filename_);
    qDebug()<<"finish saving"<<endl;
}

LutGenerator::~LutGenerator()
{
    free(bits);
}

// @brief: tone map LUT
// @param rgb: a vector with rgb data
// @ret: void
void LutGenerator::toneMap(QVector<double> & rgb)
{
    double a = 2.51;
    double b = 0.03;
    double c = 2.43;
    double d = 0.59;
    double e = 0.14;
    for (int i = 0; i < rgb.size(); i++) {
        double x = rgb[i];
        x = (x*(a*x+b))/(x*(c*x+d)+e);
        rgb[i] = qMax(x, 0.0);
        rgb[i] = qMin(x, 1.0);
    }
}

// @brief: gaussain formula
double LutGenerator::gaussain(double r, double v)
{
    double Neg_r_2 = - r * r;
    double v2 = 2.0 * v;
    return 1.0/sqrt(v2 * 3.1416) * exp(Neg_r_2 / v2);
}

// @brief: scattering formula
QVector<double> LutGenerator::scatter(double r)
{
    QVector<double> rgb(3);
    double a = LutGenerator::gaussain(r, 0.0064 * 1.414);
    double b = LutGenerator::gaussain(r, 0.0484 * 1.414);
    double c = LutGenerator::gaussain(r, 0.1870 * 1.414);
    double d = LutGenerator::gaussain(r, 0.5670 * 1.414);
    double e = LutGenerator::gaussain(r, 1.9900 * 1.414);
    double f = LutGenerator::gaussain(r, 7.4100 * 1.414);
    rgb[0] = 0.233 * a + 0.100 * b + 0.118 * c + 0.113 * d + 0.358 * e + 0.078 * f;
    rgb[1] = 0.455 * a + 0.336 * b + 0.198 * c + 0.007 * d + 0.004 * e + 0.000 * f;
    rgb[2] = 0.649 * a + 0.344 * b + 0.000 * c + 0.007 * d + 0.000 * e + 0.000 * f;
    return rgb;
}

// @brief: integral and calculate the rgb data on a point on the LUT
// @param cosTheta:
// @param skinRadius:
// @ret: rgb data on a point
QVector<double> LutGenerator::integrate(double cosTheta, double skinRadius)
{
    double theta = acos(cosTheta);
    QVector<double> totalWeight = {0.0, 0.0, 0.0};
    QVector<double> totalLight = {0.0, 0.0, 0.0};

    double angle = - M_PI / 2;
    while (angle <= M_PI / 2)
    {
        double curAngle = theta + angle;
        double diffuse = qMax(cos(curAngle), 0.0);
        diffuse = qMin(diffuse, 1.0);
        double sampleDist = abs(2.0 * skinRadius * sin(angle / 2));
        QVector<double> weights = scatter(sampleDist);
        for (int i = 0; i < 3; i++)
        {
            totalWeight[i] += weights[i];
            totalLight[i] += diffuse * weights[i];
        }
        angle += this->intensity;
    }
    QVector<double> rgb(3);
    rgb[0] = totalLight[0] / totalWeight[0];
    rgb[1] = totalLight[1] / totalWeight[1];
    rgb[2] = totalLight[2] / totalWeight[2];
    if (this->toneMapping)
    {
        toneMap(rgb);
    }
    if (this->color)
    {
        double temp = 1/2.2;
        rgb[0] = pow(rgb[0], temp);
        rgb[1] = pow(rgb[1], temp);
        rgb[2] = pow(rgb[2], temp);
    }
    rgb[0] *= 255;
    rgb[1] *= 255;
    rgb[2] *= 255;
    return rgb;
}

// @brief: draw the LUT on the window
// @param: void
// @ret: void
void LutGenerator::draw()
{

    // Define BMP Size
    const int height = this->width;
    const int width = this->width;
    const int size = height * width * 3;
    int index;

    // Part.3 Create Data
    bits = (uchar *)malloc(size);

    // Clear
    memset(bits, 0xFF, size);

    // LUT
    for (double x = 0.0; x < width; x += 0.5)
    {
        for (double y = 0.0; y < width; y += 0.5)
        {
            double uvy = y / width;
            double radius = 0.0;
            radius = 1.0 / (uvy + 0.0001) * this->radius;
            double uvx = x / width;
            uvx = -1 + 2 * uvx;
            QVector<double> rgb = integrate(uvx, radius);
            index = (width - 1 - (int)y) * width * 3 + (int)x * 3;
            bits[index + 0] = (int)rgb[0]; // Blue
            bits[index + 1] = (int)rgb[1];   // Green
            bits[index + 2] = (int)rgb[2];   // Red
        }
    }
}

// @brief: get LUT data
// @param: void
// @ret: rgb data of the LUT
uchar* LutGenerator::getData()
{
    return bits;
}


// @brief: save the LUT
// @param: void
// @ret: void
void LutGenerator::save(std::string filename)
{
    // Define BMP Size
        const int height = this->width;
        const int width = this->width;
        const int size = height * width * 3;
        int index;

        // Part.1 Create Bitmap File Header
        BITMAPFILEHEADER fileHeader;

        fileHeader.bfType = 0x4D42;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        // Part.2 Create Bitmap Info Header
        BITMAPINFOHEADER bitmapHeader = { 0 };

        bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapHeader.biHeight = height;
        bitmapHeader.biWidth = width;
        bitmapHeader.biPlanes = 1;
        bitmapHeader.biBitCount = 24;
        bitmapHeader.biSizeImage = size;
        bitmapHeader.biCompression = 0; //BI_RGB

        // Part.3 Create Data
        BYTE *bits = (BYTE *)malloc(size);

        // Clear
        memset(bits, 0xFF, size);

        // LUT
        for (double x = 0.0; x < width; x += 0.5)
        {
            for (double y = 0.0; y < width; y += 0.5)
            {

                double uvy = y / width;
                double radius = 0.0;
                if (uvy < 0.01) {
                    radius = 100;
                } else {
                    radius = 1.0 / uvy * this->radius;
                }
                double uvx = x / width;
                uvx = -1 + 2 * uvx;
                QVector<double> rgb = integrate(uvx, radius);
                index = (int)y * width * 3 + (int)x * 3;
                bits[index + 0] = (int)rgb[2]; // Blue
                bits[index + 1] = (int)rgb[1];   // Green
                bits[index + 2] = (int)rgb[0];   // Red
            }
        }

        // Write to file
        const char* path = filename.data();
        FILE *output = fopen(path, "wb");


        if (output == NULL)
        {
            printf("Cannot open file!\n");
        }
        else
        {
            fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
            fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
            fwrite(bits, size, 1, output);
            fclose(output);
        }
        delete path;
}















