#include <iostream>
#include<graphics.h>  //EasyXͼ�ο�
#include <thread>
#include <windows.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include <emmintrin.h> // ����SSE2ָ���ͷ�ļ�

#define PI 3.14159265358979

//moon-engine

//�ֱ���
int screen_1[] = { 960, 540 };
int screen_2[] = {1280, 720};
int* screen = screen_1;
//��ʼ��

//��������������
struct Mpoint {
    double x = 0, y = 0, z = 0;
};
struct Mface {
    unsigned int sequnce[3] = { 0,0,0 };
};
struct Mmesh {
    std::vector <Mpoint> vertices = {};
    std::vector <Mface> faces = {};
    std::vector <Mpoint> normal_vector = {};
};


//camera�ṹ��������
struct Camera_data{
                    //HFOVˮƽ��Ұ           VFOV��ֱ��Ұ������ûʲôʵ�����ã�
    double Horizen = 44 * PI / 180, angle_verti = 30 * PI / 180;  
    double Camera[3] = { 0, 0, 0 };  //�����λ�ã�ʵʱ��
    const double R = 0.01;   //��ƽ�浽camera�����(����ʱ���ɱ�)
    double F = 1;    //f����(Ĭ��1)
    const double front[3] = { R, 0, 0 },
                y[3] = { R, -tan(Horizen) * R, 0 },
                z[3] = { R, 0, tan(angle_verti) * R },
                move[2] = { 0, -1 };  //�ƶ�������(��׼ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨��Ϊ��׼ֵ���ɸ��ģ���
    
    //�洢ʵʱ��������
    double Forward_vec[3] = {front[0], front[1], front[2]},
                Y_vec[3] = {y[0], y[1], y[2]},
                Z_vec[3] = {z[0], z[1], z[2]},
                move_vec[2] = { move[0], move[1] };
};
//����һ�����
Camera_data Camera_1;

//������
Camera_data sharedStruct;
std::mutex camera_Remain;

//old codes
#if 0
//ʵʱ����
std::vector<double> pointRuntime;//ʵʱ��������
std::vector<int> triangleRuntime;//ʵʱ��������������
std::vector <double> NormalVectorRuntime;//��ķ�����
//�����������ڣ��������ִ�����������һ�����λ��
std::vector<int> TmpBuffer;
//ʵʱ�м���ʱ�������飨render����ִ����Ľ�����棬����������ύ����դ������
#endif


//�������
template<typename T, size_t n>
void printA(T const(&arr)[n])
{
    std::cout.precision(16);
    for (size_t i = 0; i < n; i++) {
        std::cout << arr[i] << ' ';
    }
    std::cout << std::endl;
}

// �������ڽ�����ƶ�����Ļ����
void MoveMouseToCenter() {
    // ��ȡ��Ļ�Ŀ�Ⱥ͸߶�
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ������Ļ���������
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;
    // ������ƶ�����Ļ����
    SetCursorPos(centerX, centerY);
}


//ʹ��SSE2ָ��Ŀ���ƽ����
inline double SSE2Qsqrt(double number) {
    __m128d vec;
    vec = _mm_load_sd(&number); // ����doubleֵ��SSE�Ĵ���
    vec = _mm_sqrt_pd(vec);   // ����ƽ����
    _mm_store_sd(&number, vec); // ������洢��double����
    return number;
}

//�����֮�����ƽ����
inline double KQsqrt(double number) {
    long long i;
    double x2, y;
    const double threehalfs = 1.5;

    x2 = number * 0.5;
    y = number;
    i = *reinterpret_cast<long long*>(&y); // �� double ��λģʽ����Ϊ long long
    i = 0x5fe6eb50c7b537aaLL - (i >> 1);   // what the fuck?
    y = *reinterpret_cast<double*>(&i);    // �� long long ��λģʽ���ͻ� double
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    y = y * (threehalfs - (x2 * y * y));   // 2nd iteration

    return 1/y;
}
//ѡ��ʹ�õ�ƽ��������
//ʹ������ƽ����
double (*Qsqrt)(double) = KQsqrt;

//old codes
#if 0
//old

//�������ڽ�ƽ��Ľ�������(��Ļ����)   ���          ��һ����                 �������           ���(��Ļ������)
inline void Get_Cross_Point(Camera_data &Rc, double orig[3], double pointWP[3], int output[2]) {
    //��������camera��front����������
    double nL1[] = { Rc.Forward_vec[0] - pointWP[0] +Rc.Camera[0],
                             Rc.Forward_vec[1] - pointWP[1] + Rc.Camera[1],
                             Rc.Forward_vec[2] - pointWP[2] + Rc.Camera[2] };
    //����������һ�������
    double nL2[] = { orig[0] - pointWP[0],
                             orig[1] - pointWP[1],
                             orig[2] - pointWP[2] };

    //������㵽��ƽ��ľ���
    double distance = (Rc.Forward_vec[0] * nL1[0] + Rc.Forward_vec[1] * nL1[1] + Rc.Forward_vec[2] * nL1[2]) / Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]);
    //nL2��ģ
    double Length = Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]);
    //nL2 ������nF������cosֵ
    double cosV = (Rc.Forward_vec[0] * nL2[0] + Rc.Forward_vec[1] * nL2[1] + Rc.Forward_vec[2] * nL2[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]));
    double n_plane[3];
    if (distance == 0) {
        //ǡ�þ�����ʱ����
        n_plane[0] = pointWP[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = pointWP[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = pointWP[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    }else {
        double Tmp[] = { (nL2[0] * (distance / cosV) / Length) + pointWP[0], (nL2[1] * (distance / cosV) / Length) + pointWP[1], ((nL2[2] * distance) / (cosV * Length)) + pointWP[2] };
        n_plane[0] = Tmp[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = Tmp[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = Tmp[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    };
    output[0] = static_cast<int> ((screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2]))));
    output[1] = static_cast<int> ((screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2]))));

}

// ������ķ�����                                            ������                    ����������                        �������������
inline void Get_NormalVector(std::vector <double> Point, std::vector <int> Triangle, std::vector<double>& outVector) {
    for (int i = 0; i < Triangle.size(); i += 3) {
        double na[3] = { Point.at(Triangle.at(i + 1) * 3) - Point.at(Triangle.at(i) * 3), 
            Point.at(Triangle.at(i + 1) * 3 + 1) - Point.at(Triangle.at(i) * 3 + 1), 
            Point.at(Triangle.at(i + 1) * 3 + 2) - Point.at(Triangle.at(i) * 3 + 2) 
        };
        double nb[3] = { Point[Triangle.at(i + 2) * 3] - Point[Triangle.at(i + 1) * 3], 
            Point[Triangle.at(i + 2) * 3 + 1] - Point[Triangle.at(i + 1) * 3 + 1], 
            Point[Triangle.at(i + 2) * 3 + 2] - Point[Triangle.at(i + 1) * 3 + 2] 
        };
        double nV[3] = { (nb[1] * na[2] - nb[2] * na[1]), (nb[2] * na[0] - nb[0] * na[2]), (nb[0] * na[1] - nb[1] * na[0]) };
        double Length = Qsqrt(nV[0]* nV[0] + nV[1] * nV[1] + nV[2] * nV[2]);
        outVector.push_back(nV[0] / Length);
        outVector.push_back(nV[1] / Length);
        outVector.push_back(nV[2] / Length);
    }
}

// render               ������ṹ��                                 ������                    ����������                                 ����������                                  �������
void Render_V(Camera_data &Rc, std::vector <double> Point, std::vector <int> Triangle, std::vector <double> NormalVector, std::vector<int>& out) {
    //��Ŀɼ��Լ������
    std::vector <bool> Pinfo;
    //��������Լ������
    std::vector <bool> FTinfo;
    //ת����ƽ���������ϵĶ������ʱ����
    std::vector <int> TransformTriangle;
    
    double outTmp[3] = { 0,0,0 };

    //���Ե�Ϊ��λ�Ȼ�ȡ����Ŀ���Ⱦ��Ϣ����Ⱦ�ÿɼ���(Ԥ����)
    for (int i = 0; i < Point.size(); i += 3) {
        //��ö���
        double P[3] = { Point[i], Point[i + 1], Point[i + 2] };
        double n_P[] = { P[0] - Rc.Camera[0],
                                  P[1] - Rc.Camera[1],
                                  P[2] - Rc.Camera[2] };
        double LO = Qsqrt((n_P[0]) * (n_P[0]) + (n_P[1]) * (n_P[1]) + (n_P[2]) * (n_P[2]));
        double cosV;//cos ֵ
        //����camera�غ��������
        if (LO == 0) {
            Pinfo.push_back(FALSE);
            TransformTriangle.push_back(0);
            TransformTriangle.push_back(0);
            continue;
        } else {
            cosV = (Rc.Forward_vec[0] * n_P[0] + Rc.Forward_vec[1] * n_P[1] + Rc.Forward_vec[2] * n_P[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * LO);
            //����cos
        }

        if (cosV > 0 && LO*cosV >= Rc.R) {   //�������������ǰ��ʱcosΪ��ʱ 
            //��ӵ�ɼ�����Ϣ
            Pinfo.push_back(TRUE);
            //����ӳ�䵽��ƽ�������(������������)
            outTmp[0] = (n_P[0] * Rc.R) / (LO * cosV);
            outTmp[1] = (n_P[1] * Rc.R) / (LO * cosV);
            outTmp[2] = (n_P[2] * Rc.R) / (LO * cosV);
            
            //ӳ�䵽��ƽ��������ͷ���������
            double n_plane[] = { outTmp[0] - Rc.Forward_vec[0], outTmp[1] - Rc.Forward_vec[1], outTmp[2] - Rc.Forward_vec[2] };
            
            TransformTriangle.push_back(static_cast<int> ( (screen[0] / 2) + (Rc.F* ( screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2]) ) )) );
            TransformTriangle.push_back(static_cast<int> ( (screen[1] / 2) - (Rc.F* ( screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2]) ) )) );
            //ƽ��������ӳ�䣬                                                                           ����������direcX����ϵ

        } else  {            //���ɼ����������(λ�ڽ�ƽ��֮��)
            Pinfo.push_back(FALSE);
            TransformTriangle.push_back(0);
            TransformTriangle.push_back(0);
            continue;

        }
    };
    //����Ϊ��λ��ÿ�����������Ϣ��ȡ�������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ������*(unfinish)�������㶼���ɼ��������
    for (int i = 0; i < Triangle.size(); i += 3) {
        //��������������
        double nTest[3] = { Point[Triangle[i] * 3] - Rc.Camera[0], Point[Triangle[i] * 3 + 1] - Rc.Camera[1], Point[Triangle[i] * 3 + 2] - Rc.Camera[2] };
        //���������������Ϣ(�����޳�����)
        (nTest[0] * NormalVector[i] + nTest[1] * NormalVector[i + 1] + nTest[2] * NormalVector[i + 2]) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //��ȫ�ɼ����������Ҫ)
        if (Pinfo[Triangle[i]] && Pinfo[Triangle[i + 1]] && Pinfo[Triangle[i + 2]] && FTinfo[i / 3]) {
            out.push_back(TransformTriangle[Triangle[i] * 2]);
            out.push_back(TransformTriangle[Triangle[i] * 2 + 1]);

            out.push_back(TransformTriangle[Triangle[i + 1] * 2]);
            out.push_back(TransformTriangle[Triangle[i + 1] * 2 + 1]);

            out.push_back(TransformTriangle[Triangle[i + 2] * 2]);
            out.push_back(TransformTriangle[Triangle[i + 2] * 2 + 1]);
        }else if ( FTinfo[i / 3] ) {
            //�������洦��(��Ȼ�������������ɼ���)
            int num = static_cast<int>(not Pinfo[Triangle[i]]) + static_cast<int>(not Pinfo[Triangle[i + 1]]) + static_cast<int>(not Pinfo[Triangle[i + 2]]);
            if (num == 3) continue; //�ų���ȫ���ɼ�
            //          ǰ����        �󶥵�      �м��   (��������Է����ǲ��ɼ��㻹�ǿɼ���)
            double previous[4], next[4], medium[4];
            //          �е�1                  �е�2
            int TmpClipOut_1[2], TmpClipOut_2[2];
            bool num_bool = (num == 1) ? TRUE : FALSE;

            //forѭ���ֱ����//��������Ϊ��������ɵĵ��������Ȼ������˳ʱ���˳��
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[Triangle[i + e]]) {//��ô������            //�˴������������Էֱ���1�������2���
                    medium[0] = Point[Triangle[i + e] * 3];
                    medium[1] = Point[Triangle[i + e] * 3 + 1];
                    medium[2] = Point[Triangle[i + e] * 3 + 2];
                    medium[3] = i + e;
                    previous[0] = Point[Triangle[i + (e + 5) % 3] * 3];
                    previous[1] = Point[Triangle[i + (e + 5) % 3] * 3 + 1];
                    previous[2] = Point[Triangle[i + (e + 5) % 3] * 3 + 2];
                    previous[3] = i + (e + 5) % 3;
                    next[0] = Point[Triangle[i + (e + 7) % 3] * 3];
                    next[1] = Point[Triangle[i + (e + 7) % 3] * 3 + 1];
                    next[2] = Point[Triangle[i + (e + 7) % 3] * 3 + 2];
                    next[3] = i + (e + 7) % 3;
                    break;
                }
            }
            Get_Cross_Point(Rc, previous, medium, TmpClipOut_1);
            Get_Cross_Point(Rc, next, medium, TmpClipOut_2);
            if (num_bool) {
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2 + 1]);
                out.push_back(TmpClipOut_1[0]);
                out.push_back(TmpClipOut_1[1]);
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                //�˴�һ�����ɼ����������������
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                out.push_back(TransformTriangle[(Triangle[static_cast<int>(next[3])] * 2)]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(next[3])] * 2 + 1]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2 + 1]);
                //������դ����������ӳ��ʱ���ǵ��������������ת������!!!!
            }else if(not num_bool){
                out.push_back(TmpClipOut_1[0]);
                out.push_back(TmpClipOut_1[1]);
                out.push_back(TransformTriangle[(Triangle[static_cast<int>(medium[3])] * 2)]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(medium[3])] * 2 + 1]);
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                //wait for finish
            }


            


        };
    }
}

//old

#endif

//�������ڽ�ƽ��Ľ�������new                ���                            һ����                         ��һ����                           ���(��Ļ������)
inline void Get_CrossPoint_New(const Camera_data& Rc, const Mpoint &origin, const Mpoint &process_point, Mpoint &output) {
    //��������camera��front�������
    double nL1[] = { Rc.Forward_vec[0] - process_point.x + Rc.Camera[0],
                             Rc.Forward_vec[1] - process_point.y + Rc.Camera[1],
                             Rc.Forward_vec[2] - process_point.z + Rc.Camera[2] };
    //����������һ�������
    double nL2[] = { origin.x - process_point.x,
                             origin.y - process_point.y,
                             origin.z - process_point.z };

    //������㵽��ƽ��ľ���
    double distance = (Rc.Forward_vec[0] * nL1[0] + Rc.Forward_vec[1] * nL1[1] + Rc.Forward_vec[2] * nL1[2]) / Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]);
    //nL2��ģ
    double Length = Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]);
    //nL2 ������nF������cosֵ
    double cosV = (Rc.Forward_vec[0] * nL2[0] + Rc.Forward_vec[1] * nL2[1] + Rc.Forward_vec[2] * nL2[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]));
    double n_plane[3];
    if (distance == 0) {
        //ǡ�þ�����ʱ����
        n_plane[0] = process_point.x - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = process_point.y - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = process_point.z - Rc.Forward_vec[2] - Rc.Camera[2];
    }else {
        double Tmp[] = { (nL2[0] * distance) / (cosV * Length) + process_point.x,
                                   (nL2[1] * distance) / (cosV * Length) + process_point.y,
                                  (nL2[2] * distance) / (cosV * Length) + process_point.z };
        n_plane[0] = Tmp[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = Tmp[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = Tmp[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    };
    output.x = (screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2])));
    output.y = (screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2])));
    output.z = 0;
}


// ������ķ�����New                                 mesh
inline void Get_NormalVector_New(Mmesh &cMesh) {
    Mpoint normal_vector;
    for (Mface &each_face : cMesh.faces) {
        double n_a[3] = {cMesh.vertices[each_face.sequnce[1]].x -cMesh.vertices[each_face.sequnce[0]].x ,
                                    cMesh.vertices[each_face.sequnce[1]].y - cMesh.vertices[each_face.sequnce[0]].y,
                                    cMesh.vertices[each_face.sequnce[1]].z - cMesh.vertices[each_face.sequnce[0]].z
         };
        double n_b[3] = { cMesh.vertices[each_face.sequnce[2]].x - cMesh.vertices[each_face.sequnce[1]].x,
                                    cMesh.vertices[each_face.sequnce[2]].y - cMesh.vertices[each_face.sequnce[1]].y,
                                    cMesh.vertices[each_face.sequnce[2]].z - cMesh.vertices[each_face.sequnce[1]].z
        };
        double n_V[3] = { (n_b[1] * n_a[2] - n_b[2] * n_a[1]), (n_b[2] * n_a[0] - n_b[0] * n_a[2]), (n_b[0] * n_a[1] - n_b[1] * n_a[0]) };
        double Length = Qsqrt(n_V[0] * n_V[0] + n_V[1] * n_V[1] + n_V[2] * n_V[2]);//��������ģ
        normal_vector.x = n_V[0] / Length;
        normal_vector.y = n_V[1] / Length;
        normal_vector.z = n_V[2] / Length;
        cMesh.normal_vector.push_back(normal_vector);
    }
}


// New render       ������ṹ��                          mesh                    �������mesh
void Render_New(const Camera_data& Rc, Mmesh& cMesh, Mmesh& out_mesh) {
    //��Ŀɼ��Լ������//��������Լ������
    std::vector <bool> Pinfo, FTinfo;
    //ת����ƽ���������ϵĶ������ʱ����
    std::vector <Mpoint> Transform_vertices;
    //���㷨����
    cMesh.normal_vector.clear();
    Get_NormalVector_New(cMesh);
    for (const Mpoint &each_point : cMesh.vertices) {
        //��ʱֵ
        Mpoint TmpP, n_P, outP;
        n_P.x = each_point.x - Rc.Camera[0];
        n_P.y = each_point.y - Rc.Camera[1];
        n_P.z = each_point.z - Rc.Camera[2];

        double Length = Qsqrt(n_P.x * n_P.x + n_P.y * n_P.y + n_P.z * n_P.z), cosV;
        //��������غϵ����
        if (Length == 0) {
            Pinfo.push_back(FALSE);
            Transform_vertices.push_back(TmpP);
            continue;
        } else {//cos ֵ
            cosV = (Rc.Forward_vec[0] * n_P.x + Rc.Forward_vec[1] * n_P.y + Rc.Forward_vec[2] * n_P.z) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Length);
        }
        if (cosV > 0 && Length * cosV >= Rc.R) {   //�������������ǰ��ʱcosΪ��ʱ,�������
            //��ӵ�ɼ�����Ϣ
            Pinfo.push_back(TRUE);
            //����ӳ�䵽��ƽ�������(������������)
            TmpP.x = (n_P.x * Rc.R) / (Length * cosV);
            TmpP.y = (n_P.y * Rc.R) / (Length * cosV);
            TmpP.z = (n_P.z * Rc.R) / (Length * cosV);
            //ӳ�䵽��ƽ��������ͷ���������
            double n_plane[] = { TmpP.x - Rc.Forward_vec[0], TmpP.y - Rc.Forward_vec[1], TmpP.z - Rc.Forward_vec[2] };
            outP.x = (screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2])));
            outP.y = (screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2])));
            outP.z = Length * cosV - Rc.R;   //zֵ�����洢Deepth�����������Dbuffer��
            Transform_vertices.push_back(outP);
            //ƽ��������ӳ�䣬                                                                           ����������direcX����ϵ
        } else {            //���ɼ����������(λ�ڽ�ƽ��֮��)
            Pinfo.push_back(FALSE);
            Transform_vertices.push_back(outP);//��ʹ���ɼ�ҲҪռλ
            continue;
        }
    }
    //����Ϊ��λ��ÿ�����������Ϣ��ȡ�������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ������*(unfinish)�������㶼���ɼ��������
    for (int i = 0; i < cMesh.faces.size(); ++i) {
        //��������������
        double nTest[3] = { cMesh.vertices[ cMesh.faces[i].sequnce[0] ].x - Rc.Camera[0], cMesh.vertices[ cMesh.faces[i].sequnce[0]].y - Rc.Camera[1], cMesh.vertices[ cMesh.faces[i].sequnce[0] ].z - Rc.Camera[2]};
        //���������������Ϣ(�����޳�����)
        (nTest[0] * cMesh.normal_vector[i].x + nTest[1] * cMesh.normal_vector[i].y + nTest[2] * cMesh.normal_vector[i].z) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //��ȫ�ɼ����������Ҫ)
        if (Pinfo[ cMesh.faces[i].sequnce[0] ] && Pinfo[ cMesh.faces[i].sequnce[1] ] && Pinfo[ cMesh.faces[i].sequnce[2] ] && FTinfo[i]) {
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[0]]);
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[1]]);
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[2]]);
        }
        else if (FTinfo[i]) {
            //�������洦��(��Ȼ�������������ɼ���)
            int num = static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[0] ]) + static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[1] ]) + static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[2] ]);
            if (num == 3) continue; //�ų���ȫ���ɼ�
            bool num_bool = (num == 1) ? TRUE : FALSE;
            //          ǰ����        �󶥵�      �м��   (��������Է����ǲ��ɼ��㻹�ǿɼ���)
            unsigned int previous{}, next{}, medium{};
            //          �е�1                  �е�2
            Mpoint ClipOut_1, ClipOut_2;
            //forѭ���ֱ����//��������Ϊ��������ɵĵ��������Ȼ������˳ʱ���˳��
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[ cMesh.faces[i].sequnce[e] ]) {//��ô������            //�˴������������Էֱ���1�������2���
                    medium = cMesh.faces[i].sequnce[ e ];
                    previous = cMesh.faces[i].sequnce[(e + 5) % 3];
                    next = cMesh.faces[i].sequnce[(e + 7) % 3];
                    Get_CrossPoint_New(Rc, cMesh.vertices[previous], cMesh.vertices[medium], ClipOut_1);
                    Get_CrossPoint_New(Rc, cMesh.vertices[next], cMesh.vertices[medium], ClipOut_2);
                    break;
                }
            }
            if (num_bool) {
                out_mesh.vertices.push_back(Transform_vertices[ previous ]);
                out_mesh.vertices.push_back(ClipOut_1);
                out_mesh.vertices.push_back(ClipOut_2);
                //�˴�һ�����ɼ����������������
                out_mesh.vertices.push_back(ClipOut_2);
                out_mesh.vertices.push_back(Transform_vertices[next]);
                out_mesh.vertices.push_back(Transform_vertices[previous]);
                //������դ����������ӳ��ʱ���ǵ��������������ת����
            }else{
                out_mesh.vertices.push_back(ClipOut_1);
                out_mesh.vertices.push_back(Transform_vertices[medium]);
                out_mesh.vertices.push_back(ClipOut_2);
            }
        };

    }
    //developing now

}


//�������                     Ŀ�����                    ��б�Ƕ�                    ˮƽ�Ƕ�                      ����Ƕ�             ǰ�����ƶ���//�᷽��仯��//�ݷ���仯�� 
void camera_set( Camera_data &Tc, double incline_angle, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Tc.front[0], Tc.front[1], Tc.front[2]};
    double y_tmp[] = { Tc.y[0], Tc.y[1], Tc.y[2] };
    double z_tmp[] = { Tc.z[0], Tc.z[1], Tc.z[2] };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //�м���ʱ����ֵ
    
    //�ϲ��������ƶ�����
    if (moveF != 0) {
        double n_front[] = { -Tc.Forward_vec[0],  -Tc.Forward_vec[1], -Tc.Forward_vec[2] };
        double C_front = 1 - (moveF / Tc.R);

        Tc.Camera[0] = Tc.Camera[0] - Tc.Forward_vec[0] * C_front + Tc.Forward_vec[0];
        Tc.Camera[1] = Tc.Camera[1] -Tc.Forward_vec[1] * C_front + Tc.Forward_vec[1];
        Tc.Camera[2] = Tc.Camera[2] -Tc.Forward_vec[2] * C_front + Tc.Forward_vec[2];
    }//ǰ��

    if (moveH != 0) {
        double n_horizon[] = { Tc.Camera[0] - Tc.move_vec[0], Tc.Camera[1] - Tc.move_vec[1] };
        double Lhorizon = Qsqrt((n_horizon[0])*(n_horizon[0]) + (n_horizon[1]) * (n_horizon[1]));
        double C_horizon = 1 - (moveH / Lhorizon);

        Tc.Camera[0] = n_horizon[0] * C_horizon + Tc.move_vec[0];
        Tc.Camera[1] = n_horizon[1] * C_horizon + Tc.move_vec[1];
        Tc.move_vec[0] = Tc.move_vec[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move_vec[1] + Tc.Camera[1];
    }//����

    if (moveZ != 0) {
        Tc.Camera[2] += moveZ;
    }//����

    //�����б
    if (incline_angle != 0){
        double cosV = cos(-incline_angle);
        double sinV = sin(-incline_angle);

        z_tmp[1] = Tc.z[2]*sinV + Tc.z[1]*cosV;
        z_tmp[2] = Tc.z[2]*cosV - Tc.z[1]*sinV;

        y_tmp[1] = Tc.y[2] * sinV + Tc.y[1] * cosV;
        y_tmp[2] = Tc.y[2] * cosV - Tc.y[1] * sinV;
    }

    //���������ת
    if (revolve_verti != 0){
        double cosV = cos(revolve_verti);
        double sinV = sin(revolve_verti);

        front_tmp[0] = Tc.front[0] * cosV - Tc.front[2] * sinV;
        front_tmp[2] = Tc.front[2] * cosV + Tc.front[0] * sinV;

        yt[2] = y_tmp[2] * cosV + y_tmp[0] * sinV;
        yt[0] = y_tmp[0] * cosV - y_tmp[2] * sinV;
        y_tmp[2] = yt[2];
        y_tmp[0] = yt[0];

        zt[2] = z_tmp[2] * cosV + z_tmp[0] * sinV;
        zt[0] = z_tmp[0] * cosV - z_tmp[2] * sinV;
        z_tmp[2] = zt[2];
        z_tmp[0] = zt[0];

    }

    //���ˮƽ��ת
    if (revolve_hori != 0){
        double cosV = cos(-revolve_hori);
        double sinV = sin(-revolve_hori);

        ft[1] = front_tmp[1] * cosV + front_tmp[0] * sinV;
        ft[0] = front_tmp[0] * cosV - front_tmp[1] * sinV;
        front_tmp[1] = ft[1];
        front_tmp[0] = ft[0];


        yt[1] = y_tmp[1] * cosV + y_tmp[0] * sinV;
        yt[0] = y_tmp[0] * cosV - y_tmp[1] * sinV;
        y_tmp[1] = yt[1];
        y_tmp[0] = yt[0];

        zt[1] = z_tmp[1] * cosV + z_tmp[0] * sinV;
        zt[0] = z_tmp[0] * cosV - z_tmp[1] * sinV;
        z_tmp[1] = zt[1];
        z_tmp[0] = zt[0];
     

        //�ƶ��õĸ�������ת
        Tc.move_vec[1] = Tc.move[1] * cosV + Tc.move[0] * sinV;
        Tc.move_vec[0] = Tc.move[0] * cosV - Tc.move[1] * sinV;
        Tc.move_vec[0] = Tc.move_vec[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move_vec[1] + Tc.Camera[1];
    }else if (revolve_hori == 0) {
        Tc.move_vec[0] = Tc.move[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move[1] + Tc.Camera[1];
        //��ֹ�ƶ���������㲻����
    }


    Tc.Forward_vec[0] = front_tmp[0];
    Tc.Forward_vec[1] = front_tmp[1];
    Tc.Forward_vec[2] = front_tmp[2];

    Tc.Y_vec[0] = y_tmp[0] - Tc.Forward_vec[0];
    Tc.Y_vec[1] = y_tmp[1] - Tc.Forward_vec[1];
    Tc.Y_vec[2] = y_tmp[2] - Tc.Forward_vec[2];

    Tc.Z_vec[0] = z_tmp[0] - Tc.Forward_vec[0];
    Tc.Z_vec[1] = z_tmp[1] - Tc.Forward_vec[1];
    Tc.Z_vec[2] = z_tmp[2] - Tc.Forward_vec[2];
    return;
    
    
}



struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3,1,0},{4,1,0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };
};

cube cubeData;

//old codes
#if 0
//old
//��ͼ�߳�
void draw_thread(){

    initgraph(screen[0], screen[1]);
    setbkcolor(BLACK); // ����ɫ
    settextcolor(GREEN);//������ɫ
    setlinecolor(RGB(255,255,255)); // �߿���ɫ
    setfillcolor(RGB(128, 128, 128)); 

    while (TRUE){
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();   //��֡
        //ʵʱ���鸴λ
        pointRuntime.clear();
        triangleRuntime.clear();
        NormalVectorRuntime.clear();
        //ʵʱȫ�ֶ�������������������
        for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
            pointRuntime.push_back(cubeData.Cpoint[i][0]);
            pointRuntime.push_back(cubeData.Cpoint[i][1]);
            pointRuntime.push_back(cubeData.Cpoint[i][2]);
        }
        for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
            triangleRuntime.push_back(cubeData.Ctriangle[i][0]);
            triangleRuntime.push_back(cubeData.Ctriangle[i][1]);
            triangleRuntime.push_back(cubeData.Ctriangle[i][2]);
        }
        
        //������
        camera_Remain.lock();
        //render
        Get_NormalVector(pointRuntime, triangleRuntime, NormalVectorRuntime);
        Render_V(Camera_1,pointRuntime, triangleRuntime, NormalVectorRuntime, TmpBuffer);
        //�ͷŻ�����
        camera_Remain.unlock();

        BeginBatchDraw();
        cleardevice(); // �����Ļ
        //��ʱ��դ��
        //draw triangle
        for (int i = 0; i < TmpBuffer.size(); i += 6) {
            POINT pts[] = { {TmpBuffer.at(i), TmpBuffer.at(i + 1)},{TmpBuffer.at(i + 2), TmpBuffer.at(i + 3)},{TmpBuffer.at(i + 4), TmpBuffer.at(i + 5)} };
            fillpolygon(pts, 3);
        }

        auto end = std::chrono::high_resolution_clock::now();    //��֡
        std::chrono::duration<double> elapsed_seconds = end - start;
        double Frames = 1 / elapsed_seconds.count();

        std::string title_str = "MOON_Engine_Build_version_0.3.0  Compilation_Date:";
        title_str.append(__DATE__);
        std::string Frames_str = "Frames:"+std::to_string(Frames);

        const char* title = title_str.c_str();
        const char* Frames_char = Frames_str.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) +textheight(Frames_char) + 4, "By_H  Type ESC to close this program!");

        EndBatchDraw();

        TmpBuffer.clear();
        //���û���

    }

}
#endif


void draw_thread_New(){
    initgraph(screen[0], screen[1]);
    setbkcolor(BLACK); // ����ɫ
    settextcolor(GREEN);//������ɫ
    setlinecolor(RGB(255, 255, 255)); // �߿���ɫ
    setfillcolor(RGB(128, 128, 128));
    std::vector <Mmesh> mesh_list;
    Mmesh cube_mesh;
    for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
        Mpoint& each_point = cube_mesh.vertices.emplace_back();
        each_point.x = cubeData.Cpoint[i][0];
        each_point.y = cubeData.Cpoint[i][1];
        each_point.z = cubeData.Cpoint[i][2];
    }
    for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
        Mface& each_face = cube_mesh.faces.emplace_back();
        each_face.sequnce[0] = cubeData.Ctriangle[i][0];
        each_face.sequnce[1] = cubeData.Ctriangle[i][1];
        each_face.sequnce[2] = cubeData.Ctriangle[i][2];
    }

    mesh_list.push_back(cube_mesh);

    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }
        auto start = std::chrono::high_resolution_clock::now();   //��֡
        BeginBatchDraw();
        cleardevice(); // �����Ļ
        for (Mmesh &each_mesh : mesh_list) {
            Mmesh out;
            camera_Remain.lock();
            Render_New(Camera_1, each_mesh, out);
            camera_Remain.unlock();
            //��ʱ��out_mesh��vertices��˳����������Ϊһ���棬��ʹ�ó�Աfaces
            for (unsigned int k = 0; k < out.vertices.size(); k +=3) {
                POINT p[] = { static_cast<long>(out.vertices[k].x) , static_cast<long>(out.vertices[k].y) ,static_cast<long>(out.vertices[k+1].x) , static_cast<long>(out.vertices[k+1].y) , static_cast<long>(out.vertices[k+2].x) , static_cast<long>(out.vertices[k+2].y) };
                fillpolygon(p, 3);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();    //��֡
        std::chrono::duration<double> elapsed_seconds = end - start;
        double Frames = 1 / elapsed_seconds.count();

        std::string title_str = "MOON_Engine_Build_version_0.4.0  Compilation_Date:";
        title_str.append(__DATE__);
        std::string Frames_str = "Frames:" + std::to_string(Frames);

        const char* title = title_str.c_str();
        const char* Frames_char = Frames_str.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) + textheight(Frames_char) + 4, "By_H  press ESC to close this program!");

        EndBatchDraw();
    }

}


//camera�����߳�
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };
    BOOL firstTime = TRUE;

    double angleNOW[] = { 0,0 };
    double CS[] = {0,0,0,0,0,0};
    double speed = 0.02;

    Camera_1.Camera[2] = 1;//����camera��ʼλ��ʾ��
    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(1); // ����һ��ʱ��
        GetCursorPos(&currentPos);
        
        if (firstTime) {
            firstTime = FALSE;
        }else{
            int deltaX = currentPos.x - centerPos.x;
            int deltaY = currentPos.y - centerPos.y;

            angleNOW[0] = angleNOW[0] + deltaX * 0.4;
            angleNOW[1] = angleNOW[1] - deltaY * 0.4;
            //��׼���ƻ���������

            if (angleNOW[0] > 360) {
                angleNOW[0] = angleNOW[0] - 360;
            }else if (angleNOW[0] < -360) {
                angleNOW[0] = angleNOW[0] + 360;
            }
            if (angleNOW[1] > 90) {
                angleNOW[1] = 90;
            }else if(angleNOW[1] < -90) {
                angleNOW[1] = -90;
            }

            //����camera��ͷ����
            CS[1] = angleNOW[0] * PI / 180;
            CS[2] = angleNOW[1] * PI / 180;
        }
        //������Ƶ�����
        MoveMouseToCenter();

        //������
        if (GetAsyncKeyState('W') & 0x8000){
            CS[3] = speed;
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            CS[3] = -speed;
        }else{
            CS[3] = 0;
        }
        if (GetAsyncKeyState('A') & 0x8000){
            CS[4] = -speed;
            //CS[0] = -1 * PI / 180;
        }else if (GetAsyncKeyState('D') & 0x8000){
            CS[4] = speed;
            //CS[0] = 1 * PI / 180;
        }else{
            CS[4] = 0;
            //CS[0] = 0;
        }
        if (GetKeyState(VK_LSHIFT) & 0x8000){
            CS[5] = -speed;
        }else if (GetAsyncKeyState(VK_SPACE) & 0x8000){
            CS[5] = speed;
        }else{
            CS[5] = 0;
        }

        camera_Remain.lock();
        //�佹����
        if (GetKeyState('Z') & 0x8000 && Camera_1.F < 10) {
            Camera_1.F = Camera_1.F+0.1;
        }
        if (GetAsyncKeyState('C') & 0x8000 && Camera_1.F > 0.7) {
            Camera_1.F = Camera_1.F - 0.1;
        }
        if (GetAsyncKeyState('X') & 0x8000) {
            Camera_1.F = 1;
        }
        camera_set(Camera_1,CS[0], CS[1], CS[2], CS[3], CS[4], CS[5]);
        camera_Remain.unlock();

        

    }
}


int main() {
    
    std::thread draw(draw_thread_New);
    std::thread control(control_thread);

    draw.join();
    control.join();
    return 0;
}
