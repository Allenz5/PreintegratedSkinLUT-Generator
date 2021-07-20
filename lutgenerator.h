#ifndef LUTGENERATOR_H
#define LUTGENERATOR_H
#include<QVector>
#include<QLabel>

class LutGenerator
{
private:
    int width;
    float intensity;
    float radius;
    bool color;
    bool toneMapping;
    uchar *bits;
public:
    LutGenerator(int width_, float intensity_, float radius_, bool color_, bool toneMap_);
    LutGenerator(int width_, float intensity_, float radius_, bool color_, bool toneMap_, std::string filename_);
    ~LutGenerator();
    double gaussain(double r, double v);
    QVector<double> scatter(double r);
    QVector<double> integrate(double cosTheta, double skinRadius);
    void toneMap(QVector<double> & rgb);
    void draw();
    uchar* getData();
    void save(std::string filename);
};

#endif // LUTGENERATOR_H
