#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include <SDL.h>
#include <SDL_ttf.h>
//use SDL 2

#define ENGINE_NAME "Moon_engine_Alpha_v0.5.5"
#define PI 3.14159265358979



//��������
//����
struct Vec3 {
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
    std::vector <Vec3> vertices = {};
    std::vector <Face> faces = {};
    std::vector <Vec3> normal_vectors = {};
    std::vector <Vec3> textures_uv = {};
    std::vector <Color> color = {};
};

//������ת�����mesh
struct mesh_tf {
    std::vector <Vec3> vertices_2d = {};
    std::vector <Vec3> vertices_3d = {};
    std::vector <Vec3> normal_vectors = {};
    std::vector <Vec3> textures_uv = {};
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
                           y[3] = { NearPlane, -1, 0 },
                           z[3] = { NearPlane, 0, 1 },
                           move[3] = { 0, -1, 0 };  //�ƶ�������(�̶�ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨�̶�ֵ���ɸ��ģ���

    //�洢ʵʱ��������
    Vec3 Forward_vec = { Forward[0], Forward[1], Forward[2] },
               Y_vec = { y[0], y[1], y[2] },
               Z_vec = { z[0], z[1], z[2] },
               move_vec = { move[0], move[1], move[2] };
};


struct Buffer {
    long Buffer_size[2] = {0, 0};
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
double GetLength(const Vec3 &vec);
double GetLength(const double vec[3]);//���غ���
double dot(const Vec3 &vec1, const Vec3 &vec2);
void cross(const Vec3& vec1, const Vec3& vec2, Vec3 &vec_cross);


class Transform {
private:
    inline void Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const Vec3& origin_1, const Vec3& origin_2, Vec3& output);
    inline void CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const Vec3& vertex_origin, Vec3& out);
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

    double LocateDepth(const Vec3& v0, const Vec3& v1, const Vec3& v2, double a, double b);

    //�Ż��汾
    inline void PreComputeTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);
    inline double LocateDepth_v2(double a, double b);

	inline void DrawFlatTopTriangle(const Camera_data& Receive_camera_2, Buffer& buffer, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Color& c);
    inline void DrawFlatBottomTriangle(const Camera_data& Receive_camera_2, Buffer& buffer, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Color& c);
public:
    void DrawTriangle(const Camera_data &Receive_camera_2, Buffer& buffer, const Vec3& v1, const Vec3& v2, const Vec3& v3, Color& c);


};

void Render(const Camera_data& Receive_camera, Buffer& FrameBuffer, const long screen_in[2], Mmesh& TargetMesh);
