#include "moon.h"
#include <algorithm>

//�������ڽ�ƽ��Ľ�������                          ���                                                                                                һ����                         ��һ����            ���(��Ļ������)
 inline void Transform::Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const vertex& origin_1, const vertex& origin_2, vertex& output) {
    //��������camera��front�������
    vertex vec1 = { Receive_camera.Forward_vec.x - origin_2.x + Receive_camera.Camera[0],
                                Receive_camera.Forward_vec.y - origin_2.y + Receive_camera.Camera[1],
                                Receive_camera.Forward_vec.z - origin_2.z + Receive_camera.Camera[2] };
    //����������һ�������
    vertex vec2 = { origin_1.x - origin_2.x,
                                origin_1.y - origin_2.y,
                                origin_1.z - origin_2.z };
    double vec1_dotValue = dot(Receive_camera.Forward_vec, vec1);
    double a = vec1_dotValue / dot(vec2, Receive_camera.Forward_vec);    //ת���㷨(�ѻ���)

    vertex plane_vec;
    if (vec1_dotValue == 0) {
        //ǡ�þ�����ʱ����
        plane_vec.x = origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        plane_vec.y = origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        plane_vec.z = origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    }
    else {
        /*old
                double Tmp[] = { (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x,
                                            (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y,
                                            (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z };
        */
        plane_vec.x = vec2.x * a + origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        plane_vec.y = vec2.y * a + origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        plane_vec.z = vec2.z * a + origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    };
    output.x = (screen_in[0] >> 1) + (Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Y_vec, plane_vec) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * GetLength(Receive_camera.Y_vec)));
    output.y = (screen_in[1] >> 1) - (Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Z_vec, plane_vec) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * GetLength(Receive_camera.Z_vec)));
    output.z = 0;
    return;
}



// ������ķ�����                                                  mesh
inline void Transform::Get_NormalVector(Mmesh& cMesh) {
    vertex normal_vectors;
    for (Face& each_face : cMesh.faces) {

        double vec_a[3] = { cMesh.vertices[each_face.index[1]].x - cMesh.vertices[each_face.index[0]].x ,
                                    cMesh.vertices[each_face.index[1]].y - cMesh.vertices[each_face.index[0]].y,
                                    cMesh.vertices[each_face.index[1]].z - cMesh.vertices[each_face.index[0]].z
        };
        double vec_b[3] = { cMesh.vertices[each_face.index[2]].x - cMesh.vertices[each_face.index[1]].x,
                                    cMesh.vertices[each_face.index[2]].y - cMesh.vertices[each_face.index[1]].y,
                                    cMesh.vertices[each_face.index[2]].z - cMesh.vertices[each_face.index[1]].z
        };
        double normal_vector[3] = { (vec_b[1] * vec_a[2] - vec_b[2] * vec_a[1]), (vec_b[2] * vec_a[0] - vec_b[0] * vec_a[2]), (vec_b[0] * vec_a[1] - vec_b[1] * vec_a[0]) };
        double Length = 1 / GetLength(normal_vector);//��������ģ
        normal_vectors.x = normal_vector[0] * Length;
        normal_vectors.y = normal_vector[1] * Length;
        normal_vectors.z = normal_vector[2] * Length;
        cMesh.normal_vectors.push_back(normal_vectors);
    }
    return;
}




//������װ+͸��ת��                                               �����                           �ֱ���                                ԭʼmesh                   ���mesh
void Transform::Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh) {
    std::vector <bool> Pinfo, FTinfo;//��Ŀɼ��Լ������    //�������������
    std::vector <vertex> Transformed_vertices;//ת����ƽ���������ϵĶ������ʱ����
    //���㷨����
    TargetMesh.normal_vectors.clear();
    TargetMesh.normal_vectors.reserve(TargetMesh.faces.size());
    Get_NormalVector(TargetMesh);

    for (const vertex& each_point : TargetMesh.vertices) {
        //��ʱֵ
        vertex  vec_P;
        vertex Transformed_P;
        vec_P.x = each_point.x - Receive_camera.Camera[0];
        vec_P.y = each_point.y - Receive_camera.Camera[1];
        vec_P.z = each_point.z - Receive_camera.Camera[2];

        double dotValue = dot(Receive_camera.Forward_vec, vec_P);
        //���ɼ������
        /*
        if (dotValue <= 0) {
            Pinfo.push_back(FALSE);
            Transformed_vertices.emplace_back();
            continue;
        }
        */
        if (double depth = (dotValue / Receive_camera.nearPlane); depth >= Receive_camera.nearPlane) {
            Pinfo.push_back(true);//��ӵ�ɼ�����Ϣ
            //ӳ�䵽��ƽ��������ͷ���������
            double a = (Receive_camera.nearPlane * Receive_camera.nearPlane) / dotValue;
            vertex VecPlane = { vec_P.x * a - Receive_camera.Forward_vec.x,
                                            vec_P.y * a - Receive_camera.Forward_vec.y,
                                            vec_P.z * a - Receive_camera.Forward_vec.z };
            Transformed_P.x = (screen_in[0] >> 1) + (Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Y_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * GetLength(Receive_camera.Y_vec)));
            Transformed_P.y = (screen_in[1] >> 1) - (Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Z_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * GetLength(Receive_camera.Z_vec)));
            Transformed_P.z = depth - Receive_camera.nearPlane;   //�����洢Depth��������Dbuffer��
            Transformed_vertices.emplace_back(Transformed_P);
            //ƽ��������ӳ��,����������direcX����ϵ
        }
        else {            //���ɼ��������
            Pinfo.push_back(false);
            Transformed_vertices.emplace_back(Transformed_P);//ռλ
            continue;
        }
    }
    //ֱ��������ȫ���ɼ���mesh
    if (not std::any_of(Pinfo.begin(), Pinfo.end(), [](bool value) { return value; })) return;


    //����Ϊ��λ��ÿ���Ƕ������������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ�������������㶼���ɼ��������
    for (int i = 0; i < TargetMesh.faces.size(); ++i) {
        double vecTest[3] = { TargetMesh.vertices[TargetMesh.faces[i].index[0]].x - Receive_camera.Camera[0], TargetMesh.vertices[TargetMesh.faces[i].index[0]].y - Receive_camera.Camera[1], TargetMesh.vertices[TargetMesh.faces[i].index[0]].z - Receive_camera.Camera[2] };
        //���������������Ϣ(�����޳�����)
        (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) < 0 ? FTinfo.push_back(true) : FTinfo.push_back(false);
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
            bool num_bool = (num == 1) ? true : false;
            unsigned int previous{}, next{}, medium{};//          ǰ����        �󶥵�      �м��   (��������Է����ǲ��ɼ��㻹�ǿɼ���)(����ֵ)
            vertex ClipOut_1, ClipOut_2;            //          �е�1        �е�2
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




