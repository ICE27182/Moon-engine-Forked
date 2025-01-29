#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <SDL.h>
//use SDL 2

#define ENGINE_NAME "Moon_engine_Alpha_v0.5.3"
#define PI 3.14159265358979

//��������
//����
struct Vertex {
    double x = 0, y = 0, z = 0;
};
//��
struct Face {
    unsigned int index[3] = { 0,0,0 };
};

//RGB 
struct Color {
    double R = 0, G = 0, B = 0, a = 0;
};

//ԭʼmesh
struct Mmesh {
    std::vector <Vertex> vertices = {};
    std::vector <Face> faces = {};
    std::vector <Vertex> normal_vectors = {};
    std::vector <Vertex> textures_uv = {};
    std::vector <Color> color = {};
};

//������ת�����mesh
struct mesh_tf {
    std::vector <Vertex> vertices_2d = {};
    std::vector <Vertex> vertices_3d = {};
    std::vector <Vertex> normal_vectors = {};
    std::vector <Vertex> textures_uv = {};
    std::vector <Color> color = {};
};




//camera�ṹ��������
struct Camera_data {
    //HFOVˮƽ��Ұ  
    double FOV = 40 * PI / 180;
    double F = 1;    //f����(Ĭ��1)
    const double NearPlane = 0.04;   //��ƽ�����(����ʱ���ɱ�)
    const double FarPlane = 4000;

    double CameraPos[3] = { 0, 0, 0 };  //�����λ�ã�ʵʱ��

    const double Forward[3] = { NearPlane, 0, 0 },
        y[3] = { NearPlane, -tan(FOV) * NearPlane, 0 },
        z[3] = { NearPlane, 0, tan(FOV) * NearPlane },
        move[3] = { 0, -1, 0 };  //�ƶ�������(�̶�ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨��Ϊ�̶�ֵ���ɸ��ģ���

    //�洢ʵʱ��������
    Vertex Forward_vec = { Forward[0], Forward[1], Forward[2] },
               Y_vec = { y[0], y[1], y[2] },
               Z_vec = { z[0], z[1], z[2] },
               move_vec = { move[0], move[1], move[2] };
};


struct Buffer {
    long Buffer_size[2];
    std::vector<double> RedBuffer;
    std::vector<double> GreenBuffer;
    std::vector<double> BlueBuffer;
    std::vector<double> DepthBuffer;

    void SetBuffer(long width, long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double deepth, Color c);
    Color GetPixelColor(const long x, const long y);

    bool CompareDepth_Smaller(const long x, const long y, double depth);

    double GetDepth(const long x, const long y);

};

double SSE2Qsqrt(double number);
double KQsqrt(double number);
double GetLength(const Vertex &vec);
double GetLength(const double vec[3]);//���غ���
double dot(const Vertex &vec1, const Vertex &vec2);
void cross(const Vertex& vec1, const Vertex& vec2, Vertex &vec_cross);


class Transform {
private:
    inline void Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& origin_1, const Vertex& origin_2, Vertex& output);
    inline void CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& vertex_origin, Vertex& out);
public:
    inline void Get_NormalVector(Mmesh& cMesh);
    void Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh);


};



class Graphics {
private:
    //��ʱ��
    double coeffs_[15];
    double area_triangle;

    inline double To_unLineDepth(const Camera_data& Rec_camera, double depth);

    double LocateDepth(const Vertex& v0, const Vertex& v1, const Vertex& v2, double a, double b);

    //�Ż��汾
    inline void PreComputeTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
    inline double LocateDepth_v2(double a, double b);

	inline void DrawFlatTopTriangle(const Camera_data& Receive_camera_2, Buffer& buffer, const Vertex& v0, const Vertex& v1, const Vertex& v2, const Color& c);
    inline void DrawFlatBottomTriangle(const Camera_data& Receive_camera_2, Buffer& buffer, const Vertex& v0, const Vertex& v1, const Vertex& v2, const Color& c);
public:
    void DrawTriangle(const Camera_data &Receive_camera_2, Buffer& buffer, const Vertex& v1, const Vertex& v2, const Vertex& v3, Color& c);


};

void Render(const Camera_data& Receive_camera, Buffer& FrameBuffer, const long screen_in[2], Mmesh& TargetMesh);
