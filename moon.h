#pragma once
#include <vector>
#define PI 3.14159265358979

//��������
struct vertix {
    double x = 0, y = 0, z = 0;
};

struct Face {
    unsigned int index[3] = { 0,0,0 };
};

//ԭʼmesh
struct Mmesh {
    std::vector <vertix> vertices = {};
    std::vector <Face> faces = {};
    std::vector <vertix> normal_vectors = {};
    std::vector <vertix> textures_uv = {};
};

//������ת�����mesh
struct mesh_tf {
    std::vector <vertix> vertices = {};
    std::vector <vertix> normal_vectors = {};
    std::vector <vertix> textures_uv = {};
};
//RGB 
struct Color {
    float R = 0, G = 0, B = 0 ,a = 0;
};

//camera�ṹ��������
struct Camera_data {
    //HFOVˮƽ��Ұ  
    double FOV = 40 * PI / 180;
    double Camera[3] = { 0, 0, 0 };  //�����λ�ã�ʵʱ��
    const double nearPlane = 0.01;   //��ƽ�����(����ʱ���ɱ�)
    double F = 1;    //f����(Ĭ��1)
    const double Forward[3] = { nearPlane, 0, 0 },
        y[3] = { nearPlane, -tan(FOV) * nearPlane, 0 },
        z[3] = { nearPlane, 0, tan(FOV) * nearPlane },
        move[3] = { 0, -1, 0 };  //�ƶ�������(�̶�ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨��Ϊ�̶�ֵ���ɸ��ģ���

    //�洢ʵʱ��������
    vertix Forward_vec = { Forward[0], Forward[1], Forward[2] },
        Y_vec = { y[0], y[1], y[2] },
        Z_vec = { z[0], z[1], z[2] },
        move_vec = { move[0], move[1], move[2] };
};

double SSE2Qsqrt(double number);
double KQsqrt(double number);

class vectorMath {
public:
    double GetLength(const vertix &vec);
    double GetLength(const double vec[3]);//���غ���
    double dot(const vertix &vec1, const vertix &vec2);
    void cross(const vertix& vec1, const vertix& vec2, vertix &vec_cross);
};





class Graphic {
private:
    static unsigned long Buffer_size[2];
    static std::vector<double> RedBuffer;
    static std::vector<double> GreenBuffer;
    static std::vector<double> BlueBuffer;

    static std::vector<double> DepthBuffer;

public:



    void SetBuffer(unsigned long width, unsigned long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double deepth, Color c);
    Color GetPixelColor(const long x, const long y);

	void DrawFlatTopTriangle(const vertix& v0, const vertix& v1, const vertix& v2, const Color& c);
    void DrawFlatBottomTriangle(const vertix& v0, const vertix& v1, const vertix& v2, const Color& c);
	void DrawTriangle(const vertix& v1, const vertix& v2, const vertix& v3, Color& c);


};

