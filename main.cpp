#include "moon.h"

#include <algorithm>
#include<graphics.h>  //EasyXͼ�ο�
#include <iostream>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

vectorMath vecMath;
Graphic Graphic_func;
//moon-engine

//�ֱ���
unsigned long screen_1[] = { 960, 540 };
unsigned long screen_2[] = {1280, 720};
unsigned long* ptrScreen = screen_1;
//��ʼ��

//����һ�����
Camera_data Camera_1;

//������
Camera_data sharedStruct;
std::mutex CameraData_Remain;

#if 0
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
#endif

class Transform {

private:
//�������ڽ�ƽ��Ľ�������new                ���                                                                                 һ����                         ��һ����            ���(��Ļ������)
inline void Get_CrossPoint(const Camera_data& Receive_camera,unsigned const long screen_in[2], const vertix& origin_1, const vertix& origin_2, vertix_tf& output) {
    //��������camera��front�������
    vertix vec1 = { Receive_camera.Forward_vec.x - origin_2.x + Receive_camera.Camera[0],
                             Receive_camera.Forward_vec.y - origin_2.y + Receive_camera.Camera[1],
                             Receive_camera.Forward_vec.z - origin_2.z + Receive_camera.Camera[2] };
    //����������һ�������
    vertix vec2 = { origin_1.x - origin_2.x,
                             origin_1.y - origin_2.y,
                             origin_1.z - origin_2.z };
    double vec1_dotValue = vecMath.dot(Receive_camera.Forward_vec, vec1);
    double a = vec1_dotValue / vecMath.dot(vec2, Receive_camera.Forward_vec);    //ת���㷨(�ѻ���)

    vertix plane_vec;
    if (vec1_dotValue == 0) {
        //ǡ�þ�����ʱ����
        plane_vec.x = origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        plane_vec.y = origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        plane_vec.z = origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    }else {
        /*old
               double Tmp[] = { (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x,
                                           (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y,
                                           (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z };
        */
        plane_vec.x = vec2.x * a + origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        plane_vec.y = vec2.y * a+ origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        plane_vec.z = vec2.z * a + origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    };
    output.x = static_cast<long>( (screen_in[0] >>1) + (Receive_camera.F * (screen_in[0] >> 1) * vecMath.dot(Receive_camera.Y_vec, plane_vec) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Y_vec) )) );
    output.y = static_cast<long>( (screen_in[1] >>1) - (Receive_camera.F * (screen_in[0] >> 1) * vecMath.dot(Receive_camera.Z_vec, plane_vec) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Z_vec) )) );
    output.depth = 0;
    return;
}

public:
// ������ķ�����New                               mesh
inline void Get_NormalVector(Mmesh &cMesh) {
    vertix normal_vectors;
    for (Face &each_face : cMesh.faces) {
        
        double vec_a[3] = {cMesh.vertices[each_face.index[1]].x -cMesh.vertices[each_face.index[0]].x ,
                                    cMesh.vertices[each_face.index[1]].y - cMesh.vertices[each_face.index[0]].y,
                                    cMesh.vertices[each_face.index[1]].z - cMesh.vertices[each_face.index[0]].z
         };
        double vec_b[3] = { cMesh.vertices[each_face.index[2]].x - cMesh.vertices[each_face.index[1]].x,
                                    cMesh.vertices[each_face.index[2]].y - cMesh.vertices[each_face.index[1]].y,
                                    cMesh.vertices[each_face.index[2]].z - cMesh.vertices[each_face.index[1]].z
        };
        double normal_vector[3] = {(vec_b[1] * vec_a[2] - vec_b[2] * vec_a[1]), (vec_b[2] * vec_a[0] - vec_b[0] * vec_a[2]), (vec_b[0] * vec_a[1] - vec_b[1] * vec_a[0])};
        double Length = 1 / vecMath.GetLength(normal_vector);//��������ģ
        normal_vectors.x = normal_vector[0] * Length;
        normal_vectors.y = normal_vector[1] * Length;
        normal_vectors.z = normal_vector[2] * Length;
        cMesh.normal_vectors.push_back(normal_vectors);
    }
    return;
}

// New                              �����                                                    �ֱ���                                     mesh                     �������mesh
void Perspective(const Camera_data& Receive_camera, unsigned const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh) {
    std::vector <bool> Pinfo, FTinfo;//��Ŀɼ��Լ������    //�������������
    std::vector <vertix_tf> Transformed_vertices;//ת����ƽ���������ϵĶ������ʱ����
    //���㷨����
    TargetMesh.normal_vectors.clear();
    TargetMesh.normal_vectors.reserve(TargetMesh.faces.size());
    Get_NormalVector(TargetMesh);

    for (const vertix& each_point : TargetMesh.vertices) {
        //��ʱֵ
        vertix  vec_P;
        vertix_tf Transformed_P;
        vec_P.x = each_point.x - Receive_camera.Camera[0];
        vec_P.y = each_point.y - Receive_camera.Camera[1];
        vec_P.z = each_point.z - Receive_camera.Camera[2];

        double dotValue = vecMath.dot(Receive_camera.Forward_vec, vec_P);
        //���ɼ������
        if (dotValue <= 0) {
            Pinfo.push_back(FALSE);
            Transformed_vertices.emplace_back();
            continue;
        }
        if (double depth = (dotValue / Receive_camera.nearPlane); depth >= Receive_camera.nearPlane) {
            Pinfo.push_back(TRUE);//��ӵ�ɼ�����Ϣ
            //ӳ�䵽��ƽ��������ͷ���������
            double a = (Receive_camera.nearPlane * Receive_camera.nearPlane) / dotValue;
            vertix VecPlane = { vec_P.x * a - Receive_camera.Forward_vec.x,
                                           vec_P.y * a - Receive_camera.Forward_vec.y,
                                           vec_P.z * a - Receive_camera.Forward_vec.z };
            Transformed_P.x = static_cast<long>((screen_in[0] >> 1) + (Receive_camera.F * (screen_in[0] >> 1) * vecMath.dot(Receive_camera.Y_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Y_vec))));
            Transformed_P.y = static_cast<long>((screen_in[1] >> 1) - (Receive_camera.F * (screen_in[0] >> 1) * vecMath.dot(Receive_camera.Z_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Z_vec))));
            Transformed_P.depth = depth - Receive_camera.nearPlane;   //�����洢Depth��������Dbuffer��
            Transformed_vertices.emplace_back(Transformed_P);
            //ƽ��������ӳ�䣬                                                                           ����������direcX����ϵ
        }
        else {            //�����������
            Pinfo.push_back(FALSE);
            Transformed_vertices.emplace_back(Transformed_P);//ռλ
            continue;
        }
    }
    //ֱ��������ȫ���ɼ���mesh
    if (not std::any_of(Pinfo.begin(), Pinfo.end(), [](bool value) { return value; })) return;


    //����Ϊ��λ��ÿ���Ƕ������������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ�������������㶼���ɼ��������
    for (int i = 0; i < TargetMesh.faces.size(); ++i) {
        //��������������
        double vecTest[3] = { TargetMesh.vertices[TargetMesh.faces[i].index[0]].x - Receive_camera.Camera[0], TargetMesh.vertices[TargetMesh.faces[i].index[0]].y - Receive_camera.Camera[1], TargetMesh.vertices[TargetMesh.faces[i].index[0]].z - Receive_camera.Camera[2] };
        //���������������Ϣ(�����޳�����)
        (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //��ȫ�ɼ����������Ҫ)
        if (Pinfo[TargetMesh.faces[i].index[0]] && Pinfo[TargetMesh.faces[i].index[1]] && Pinfo[TargetMesh.faces[i].index[2]] && FTinfo[i]) {
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[2]]);

            out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
        }
        else if (FTinfo[i]) {
            //�������洦��(��Ȼ�������������ɼ���)
            int num = static_cast<int>(not Pinfo[TargetMesh.faces[i].index[0]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[1]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[2]]);
            if (num == 3) continue; //�ų���ȫ���ɼ�
            bool num_bool = (num == 1) ? TRUE : FALSE;
            unsigned int previous{}, next{}, medium{};//          ǰ����        �󶥵�      �м��   (��������Է����ǲ��ɼ��㻹�ǿɼ���)(����ֵ)
            vertix_tf ClipOut_1, ClipOut_2;            //          �е�1        �е�2
            //forѭ���ֱ����//��������Ϊ��������ɵĵ��������Ȼ������˳ʱ���˳��
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[TargetMesh.faces[i].index[e]]) {//��ô������            //�˴����������ֱ���1�������2���
                    medium = TargetMesh.faces[i].index[e];
                    previous = TargetMesh.faces[i].index[(e + 5) % 3];
                    next = TargetMesh.faces[i].index[(e + 7) % 3];
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[previous], TargetMesh.vertices[medium], ClipOut_1);
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[next], TargetMesh.vertices[medium], ClipOut_2);
                    break;
                }
            }
            if (num_bool) {
                out_mesh.vertices.emplace_back(Transformed_vertices[previous]);
                out_mesh.vertices.emplace_back(ClipOut_1);
                out_mesh.vertices.emplace_back(ClipOut_2);
                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                //�˴�һ�����ɼ����������������
                out_mesh.vertices.emplace_back(ClipOut_2);
                out_mesh.vertices.emplace_back(Transformed_vertices[next]);
                out_mesh.vertices.emplace_back(Transformed_vertices[previous]);
                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                //������դ����������ӳ��ʱ���ǵ��������������ת����
            }
            else {
                out_mesh.vertices.emplace_back(ClipOut_1);
                out_mesh.vertices.emplace_back(Transformed_vertices[medium]);
                out_mesh.vertices.emplace_back(ClipOut_2);
                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
            }
        };

    }
    return;
    //developing now
}


};
Transform Tf_func;

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

//�������                     Ŀ�����                       ��б�Ƕ�                    ˮƽ�Ƕ�                      ����Ƕ�          ǰ�����ƶ���//�᷽��仯��//�ݷ���仯�� 
void camera_set( Camera_data &Target_camera, double revolve_incline, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Target_camera.Forward[0], Target_camera.Forward[1], Target_camera.Forward[2]};
    double y_tmp[] = { Target_camera.y[0], Target_camera.y[1], Target_camera.y[2] };
    double z_tmp[] = { Target_camera.z[0], Target_camera.z[1], Target_camera.z[2] };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //�м���ʱ����ֵ
    
    //�ϲ��������ƶ�����
    if (moveF != 0) {
        double a_front = 1 - (moveF / Target_camera.nearPlane);

        Target_camera.Camera[0] = Target_camera.Camera[0] - Target_camera.Forward_vec.x * a_front + Target_camera.Forward_vec.x;
        Target_camera.Camera[1] = Target_camera.Camera[1] -Target_camera.Forward_vec.y * a_front + Target_camera.Forward_vec.y;
        Target_camera.Camera[2] = Target_camera.Camera[2] -Target_camera.Forward_vec.z * a_front + Target_camera.Forward_vec.z;
    }//ǰ��

    if (moveH != 0) {
        vertix horizon_vec = { Target_camera.Camera[0] - Target_camera.move_vec.x, Target_camera.Camera[1] - Target_camera.move_vec.y };
        double a_horizon = 1 - (moveH / vecMath.GetLength(horizon_vec));

        Target_camera.Camera[0] = horizon_vec.x * a_horizon + Target_camera.move_vec.x;
        Target_camera.Camera[1] = horizon_vec.y * a_horizon + Target_camera.move_vec.y;
        Target_camera.move_vec.x = Target_camera.move_vec.x + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move_vec.y + Target_camera.Camera[1];
    }//����

    if (moveZ != 0) {
        Target_camera.Camera[2] += moveZ;
    }//����

    //�����б
    if (revolve_incline != 0){
        double cosV = cos(-revolve_incline);
        double sinV = sin(-revolve_incline);

        z_tmp[1] = Target_camera.z[2]*sinV + Target_camera.z[1]*cosV;
        z_tmp[2] = Target_camera.z[2]*cosV - Target_camera.z[1]*sinV;

        y_tmp[1] = Target_camera.y[2] * sinV + Target_camera.y[1] * cosV;
        y_tmp[2] = Target_camera.y[2] * cosV - Target_camera.y[1] * sinV;
    }

    //���������ת
    if (revolve_verti != 0){
        double cosV = cos(revolve_verti);
        double sinV = sin(revolve_verti);

        front_tmp[0] = Target_camera.Forward[0] * cosV - Target_camera.Forward[2] * sinV;
        front_tmp[2] = Target_camera.Forward[2] * cosV + Target_camera.Forward[0] * sinV;

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
        Target_camera.move_vec.y = Target_camera.move[1] * cosV + Target_camera.move[0] * sinV;
        Target_camera.move_vec.x = Target_camera.move[0] * cosV - Target_camera.move[1] * sinV;
        Target_camera.move_vec.x = Target_camera.move_vec.x + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move_vec.y + Target_camera.Camera[1];
    } else {
        Target_camera.move_vec.x = Target_camera.move[0] + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move[1] + Target_camera.Camera[1];
    }


    Target_camera.Forward_vec.x = front_tmp[0];
    Target_camera.Forward_vec.y = front_tmp[1];
    Target_camera.Forward_vec.z = front_tmp[2];

    Target_camera.Y_vec.x = y_tmp[0] - Target_camera.Forward_vec.x;
    Target_camera.Y_vec.y = y_tmp[1] - Target_camera.Forward_vec.y;
    Target_camera.Y_vec.z = y_tmp[2] - Target_camera.Forward_vec.z;

    Target_camera.Z_vec.x = z_tmp[0] - Target_camera.Forward_vec.x;
    Target_camera.Z_vec.y = z_tmp[1] - Target_camera.Forward_vec.y;
    Target_camera.Z_vec.z = z_tmp[2] - Target_camera.Forward_vec.z;
    return;
    
    
}


//old
struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3,1,0},{4,1,0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };
};
cube cubeData;




void Render_thread() {
    initgraph(ptrScreen[0], ptrScreen[1]);
    setbkcolor(BLACK);
    settextcolor(GREEN);//������ɫ
    setlinecolor(RGB(255, 255, 255)); // �߿���ɫ
    setfillcolor(RGB(128, 128, 128));

    std::vector <Mmesh> mesh_list;
    //cube testѹ������
    for (int h = -2; h < 0;h +=2) {
    for (int t = -20; t < 20; t += 2) {
        for (int w = -20; w < 20;w +=2) {
            Mmesh cube_mesh;
            for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
                vertix& each_point = cube_mesh.vertices.emplace_back();
                each_point.x = cubeData.Cpoint[i][0] + t;
                each_point.y = cubeData.Cpoint[i][1] + w;
                each_point.z = cubeData.Cpoint[i][2] + h ;
            }
            for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
                Face& each_face = cube_mesh.faces.emplace_back();
                each_face.index[0] = cubeData.Ctriangle[i][0];
                each_face.index[1] = cubeData.Ctriangle[i][1];
                each_face.index[2] = cubeData.Ctriangle[i][2];
            }

            mesh_list.emplace_back(cube_mesh);
        }
    }
    }

    //֡��������
    Graphic_func.SetBuffer(ptrScreen[0], ptrScreen[1]);

    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }
        unsigned int faces_num = 0;
        double Frames;
        auto start = std::chrono::high_resolution_clock::now();   //��֡

        BeginBatchDraw();
        cleardevice(); // �����Ļ

        auto PSTcosttime_0 = std::chrono::high_resolution_clock::now();

        mesh_tf out;
        CameraData_Remain.lock();
        for (Mmesh& each_mesh : mesh_list) {
            Tf_func.Perspective(Camera_1, ptrScreen, each_mesh, out);
            //��ʱ��out_mesh��vertices��˳����������Ϊһ���棬��ʹ�ó�Աfaces
        }
        CameraData_Remain.unlock();

        auto PSTcosttime_1 = std::chrono::high_resolution_clock::now();

        /*
        

         //new test
        Graphic_func.CleanBuffer();
        
            for (unsigned int k = 0; k < out.vertices.size(); k+=3) {
            Color c = {0.1, 0.1, 0.1};

            Graphic_func.DrawTriangle(out.vertices[k], out.vertices[k+1], out.vertices[k+2], c);

        }
        
                */

        //here is a bug
        



        //��ʱʵ�ֹ�դ��
        for (unsigned int k = 0; k < out.vertices.size(); k += 3) {
            POINT triangle[] = { out.vertices[k].x , out.vertices[k].y ,out.vertices[k + 1].x , out.vertices[k + 1].y , out.vertices[k + 2].x , out.vertices[k + 2].y };
            fillpolygon(triangle, 3);
        }
        
        auto end = std::chrono::high_resolution_clock::now();    //��֡
        std::chrono::duration<double> elapsed_seconds = end - start;
        Frames = 1/elapsed_seconds.count();
        faces_num = out.vertices.size() / 3;

        std::chrono::duration<double> PSTcosttime = PSTcosttime_1 - PSTcosttime_0;
        double PSTcosttime_process = 1/PSTcosttime.count();


        std::string title_str = "MOON_Engine_Build_version_0.4.3  Compilation_Date:";
        title_str.append(__DATE__);
        std::string info = "FPS: " + std::to_string(Frames);
        std::string info2 = ("  PST_origin_FPS:" + std::to_string(PSTcosttime_process));
        std::string Faces_number = "  Faces: " + std::to_string(faces_num);
        info.append(info2);
        info.append(Faces_number);

        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) + textheight(Frames_char) + 4, "By_H  press ESC to close this program!");
        fillcircle(ptrScreen[0] / 2, ptrScreen[1] / 2, 2);

        EndBatchDraw();
    }

}


//camera�����߳�
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };
    BOOL firstTime = TRUE;

    double angleCurrent[] = { 0,0 };
    double Control[] = {0,0,0,0,0,0};
    double speed = 0.03;

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

            angleCurrent[0] = angleCurrent[0] + deltaX * 0.4;
            angleCurrent[1] = angleCurrent[1] - deltaY * 0.4;
            //��׼������

            if (angleCurrent[0] > 360) {
                angleCurrent[0] = angleCurrent[0] - 360;
            }if (angleCurrent[0] < -360) {
                angleCurrent[0] = angleCurrent[0] + 360;
            }
            if (angleCurrent[1] > 90) {
                angleCurrent[1] = 90;
            }if(angleCurrent[1] < -90) {
                angleCurrent[1] = -90;
            }

            //����camera��ͷ����
            Control[1] = angleCurrent[0] * PI / 180;
            Control[2] = angleCurrent[1] * PI / 180;
        }
        MoveMouseToCenter();

        //������

        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
            speed =0.1;
        }else{
            speed = 0.03;
        }

        if (GetAsyncKeyState('W') & 0x8000){
            Control[3] = speed;
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            Control[3] = -speed;
        }else{
            Control[3] = 0;
        }
        if (GetAsyncKeyState('A') & 0x8000){
            Control[4] = -speed;
            //Control[0] = Control[0] + 1 * PI / 180;
        }else if (GetAsyncKeyState('D') & 0x8000){
            Control[4] = speed;
            //Control[0] = Control[0] - 1 * PI / 180;
        }else{
            Control[4] = 0;
            //Control[0] = 0;
        }

        if (GetKeyState(VK_LSHIFT) & 0x8000){
            Control[5] = -speed;
        }else if (GetAsyncKeyState(VK_SPACE) & 0x8000){
            Control[5] = speed;
        }else{
            Control[5] = 0;
        }

        CameraData_Remain.lock();
        //�佹����
        if (GetKeyState('Z') & 0x8000 && Camera_1.F < 10) {
            Camera_1.F = Camera_1.F+0.1;
        }
        if (GetAsyncKeyState('C') & 0x8000 && Camera_1.F > 0.5) {
            Camera_1.F = Camera_1.F - 0.1;
        }
        if (GetAsyncKeyState('X') & 0x8000) {
            Camera_1.F = 1;
        }

        if (GetAsyncKeyState('J') & 0x8000) {
            Control[0] = Control[0] + 1 * PI / 180;
        }
        else if (GetAsyncKeyState('K') & 0x8000) {
            Control[0] = Control[0] - 1 * PI / 180;
        }
        else if (GetAsyncKeyState('L') & 0x8000) {
            Control[0] = 0;
        }
        camera_set(Camera_1,Control[0], Control[1], Control[2], Control[3], Control[4], Control[5]);
        CameraData_Remain.unlock();


    }
}


int main() {
    std::thread render(Render_thread);
    std::thread control(control_thread);

    render.join();
    control.join();
    return 0;
}
