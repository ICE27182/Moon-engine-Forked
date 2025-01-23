#pragma once
#include <vector>
#define PI 3.14159265358979

//��������
//����
struct vertex {
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
    std::vector <vertex> vertices = {};
    std::vector <Face> faces = {};
    std::vector <vertex> normal_vectors = {};
    std::vector <vertex> textures_uv = {};
    std::vector <Color> color = {};
};

//������ת�����mesh
struct mesh_tf {
    std::vector <vertex> vertices_2d = {};
    std::vector <vertex> vertices_3d = {};
    std::vector <vertex> normal_vectors = {};
    std::vector <vertex> textures_uv = {};
    std::vector <Color> color = {};
};


//camera�ṹ��������
struct Camera_data {
    //HFOVˮƽ��Ұ  
    double FOV = 40 * PI / 180;
    double Camera[3] = { 0, 0, 0 };  //�����λ�ã�ʵʱ��
    const double NearPlane = 0.04;   //��ƽ�����(����ʱ���ɱ�)
    const double FarPlane = 4000;
    double F = 1;    //f����(Ĭ��1)
    const double Forward[3] = { NearPlane, 0, 0 },
        y[3] = { NearPlane, -tan(FOV) * NearPlane, 0 },
        z[3] = { NearPlane, 0, tan(FOV) * NearPlane },
        move[3] = { 0, -1, 0 };  //�ƶ�������(�̶�ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨��Ϊ�̶�ֵ���ɸ��ģ���

    //�洢ʵʱ��������
    vertex Forward_vec = { Forward[0], Forward[1], Forward[2] },
               Y_vec = { y[0], y[1], y[2] },
               Z_vec = { z[0], z[1], z[2] },
               move_vec = { move[0], move[1], move[2] };
};

double SSE2Qsqrt(double number);
double KQsqrt(double number);
double GetLength(const vertex &vec);
double GetLength(const double vec[3]);//���غ���
double dot(const vertex &vec1, const vertex &vec2);
void cross(const vertex& vec1, const vertex& vec2, vertex &vec_cross);


class Transform {
private:
    inline void Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const vertex& origin_1, const vertex& origin_2, vertex& output);
    inline void CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const vertex& vertex_origin, vertex& out);
public:
    inline void Get_NormalVector(Mmesh& cMesh);
    void Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh);


};




class Graphics {
private:
	inline void DrawFlatTopTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c);
    inline void DrawFlatBottomTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c);
public:
    static long Buffer_size[2];
    static std::vector<double> RedBuffer;
    static std::vector<double> GreenBuffer;
    static std::vector<double> BlueBuffer;

    static std::vector<double> DepthBuffer;

    inline double To_unLineDepth(Camera_data& Rec_camera, double& depth);

    void SetBuffer(long width, long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double deepth, Color c);
    Color GetPixelColor(const long x, const long y);


	void DrawTriangle(const vertex& v1, const vertex& v2, const vertex& v3, Color& c);


};

