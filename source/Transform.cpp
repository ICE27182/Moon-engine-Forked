#include "moon.h"


//�������ڽ�ƽ��Ľ�������                          ���                                                                                                һ����                         ��һ����            ���(��Ļ������)
 inline void Transform::Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& origin_1, const Vertex& origin_2, Vertex& out_vec_3d) {
    //��������camera��front�������
    Vertex vec1 = { Receive_camera.Forward_vec.x - origin_2.x + Receive_camera.CameraPos[0],
                                Receive_camera.Forward_vec.y - origin_2.y + Receive_camera.CameraPos[1],
                                Receive_camera.Forward_vec.z - origin_2.z + Receive_camera.CameraPos[2] };
    //����������һ�������
    Vertex vec2 = { origin_1.x - origin_2.x,
                                origin_1.y - origin_2.y,
                                origin_1.z - origin_2.z };
    //double vec1_dotValue = dot(Receive_camera.Forward_vec, vec1);
    double a = dot(Receive_camera.Forward_vec, vec1) / dot(vec2, Receive_camera.Forward_vec);    //ת���㷨(�ѻ���)
    if (a == 0) {
        //ǡ�þ�����ʱ����
        out_vec_3d.x = origin_2.x - Receive_camera.CameraPos[0];
        out_vec_3d.y = origin_2.y - Receive_camera.CameraPos[1];
        out_vec_3d.z = origin_2.z - Receive_camera.CameraPos[2];
    }
    else {
        /*old
                double Tmp[] = { (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x,
                                            (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y,
                                            (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z };
        */
        out_vec_3d.x = vec2.x * a + origin_2.x - Receive_camera.CameraPos[0];
        out_vec_3d.y = vec2.y * a + origin_2.y - Receive_camera.CameraPos[1];
        out_vec_3d.z = vec2.z * a + origin_2.z - Receive_camera.CameraPos[2];
    };
    return;
}



// ������ķ�����                                                  mesh
inline void Transform::Get_NormalVector(Mmesh& cMesh) {
    Vertex normal_vectors;
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


inline void Transform::CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& vertex_origin, Vertex& out) {
    Vertex VecPlane = { vertex_origin.x - Receive_camera.Forward_vec.x,
                                    vertex_origin.y - Receive_camera.Forward_vec.y,
                                    vertex_origin.z - Receive_camera.Forward_vec.z };
    double b = (Receive_camera.F * (screen_in[0] >> 1))  / (tan(Receive_camera.FOV) * Receive_camera.NearPlane);
    /*old 
    * ����ԭ��
    out.x = (screen_in[0] >> 1) + ( Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Y_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane * GetLength(Receive_camera.Y_vec)));
    out.y = (screen_in[1] >> 1) - ( Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Z_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane * GetLength(Receive_camera.Z_vec)));
    */
    out.x = (screen_in[0] >> 1) + ((b * dot(Receive_camera.Y_vec, VecPlane)) / GetLength(Receive_camera.Y_vec));
    out.y = (screen_in[1] >> 1) - (b * dot(Receive_camera.Z_vec, VecPlane) / GetLength(Receive_camera.Z_vec));
    //ƽ��������ӳ��,����������direcX����ϵ

}


//��������װ+͸��ת��                                               �����                           �ֱ���                                ԭʼmesh                   ���mesh
void Transform::Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh) {
    std::vector <bool> Pinfo;//��Ŀɼ��Լ������
    std::vector <Vertex> Transformed_vertices;//���㻺�����
    //���㷨����
    TargetMesh.normal_vectors.clear();
    TargetMesh.normal_vectors.reserve(TargetMesh.faces.size());
    Get_NormalVector(TargetMesh);

    for (const Vertex& each_point : TargetMesh.vertices) {
        //��ʱֵ
        Vertex  vec_P;
        Vertex Transformed_P;
        vec_P.x = each_point.x - Receive_camera.CameraPos[0];
        vec_P.y = each_point.y - Receive_camera.CameraPos[1];
        vec_P.z = each_point.z - Receive_camera.CameraPos[2];

        double dotValue = dot(Receive_camera.Forward_vec, vec_P);
        if (double depth = (dotValue / Receive_camera.NearPlane); depth >= Receive_camera.NearPlane) {
            Pinfo.push_back(true);//��ӵ�ɼ�����Ϣ

            double a = (Receive_camera.NearPlane * Receive_camera.NearPlane) / dotValue;             //�˴��㷨�Ѿ�����
            vec_P = { vec_P.x * a , vec_P.y * a, vec_P.z * a };                                                                     //����ӳ�䵽��άcamera_sapceƽ�������
            
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, vec_P, Transformed_P);            //ת��Ϊ��Ļ����
            Transformed_P.z = 1/ depth;     //�����洢camera_Space����Depth��������Dbuffer��       //�����
 
        } else { 
             //���ɼ��������
            Pinfo.push_back(false);
        }
        Transformed_vertices.emplace_back(Transformed_P);//��ת����ռλ��vertex��������
    }
    
    if (not std::any_of(Pinfo.begin(), Pinfo.end(), [](bool value) { return value; })) return;//ֱ��������ȫ���ɼ���mesh

    //����Ϊ��λ��ÿ���Ƕ������������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ�������������㶼���ɼ��������
    for (int i = 0; i < TargetMesh.faces.size(); ++i) {
        double vecTest[3] = { TargetMesh.vertices[TargetMesh.faces[i].index[0]].x - Receive_camera.CameraPos[0], TargetMesh.vertices[TargetMesh.faces[i].index[0]].y - Receive_camera.CameraPos[1], TargetMesh.vertices[TargetMesh.faces[i].index[0]].z - Receive_camera.CameraPos[2] };
        //�޳�����
        //bool can_see = (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) < 0 ? true : false;
        if ( (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) >= 0) continue;
        //��ȫ�ɼ������

        if (Pinfo[TargetMesh.faces[i].index[0]] && Pinfo[TargetMesh.faces[i].index[1]] && Pinfo[TargetMesh.faces[i].index[2]] ) {
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[2]]);

            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[2]]);

            out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);

            out_mesh.color.emplace_back(TargetMesh.color[i]);
        }
        else {
            //�������洦��(��Ȼ�������������ɼ���)
            int num = static_cast<int>(not Pinfo[TargetMesh.faces[i].index[0]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[1]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[2]]);
            if (num == 3) continue;
            bool num_bool = (num == 1) ? true : false;
            
            unsigned int previous{}, next{}, medium{};//          previous Vertex, next Vertex, medium Vertex    (��������Է����ǲ��ɼ��㻹�ǿɼ���)(����ֵ)
            Vertex ClipOut_3d_1, ClipOut_3d_2;            //          3d Vertex 1        3d Vertex 2
            Vertex ClipOut_1, ClipOut_2;            //          Vertex 1        Vertex 2
            //Ϊ��������ɵĵ��������Ȼ������˳ʱ���˳��
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[TargetMesh.faces[i].index[e]]) {//��ô������            //�˴����������ֱ���1�������2���
                    medium = TargetMesh.faces[i].index[e];
                    previous = TargetMesh.faces[i].index[(e + 5) % 3];
                    next = TargetMesh.faces[i].index[(e + 7) % 3];
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[previous], TargetMesh.vertices[medium], ClipOut_3d_1);
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[next], TargetMesh.vertices[medium], ClipOut_3d_2);
                    break;
                }
            }
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_1, ClipOut_1);
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_2, ClipOut_2);

            ClipOut_1.z = 1 / Receive_camera.NearPlane; //�����
            ClipOut_2.z = 1 / Receive_camera.NearPlane;
            if (num_bool) {
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[previous]);
                out_mesh.vertices_2d.emplace_back(ClipOut_1);
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[previous]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_1);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);

                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
                
                //�˴�����ڶ�����
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[next]);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[previous]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[next]);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[previous]);
                
                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
                //������դ����������ӳ��ʱ���ǵ������uv����ת����
            }
            else {
                //���е�һ��������
                out_mesh.vertices_2d.emplace_back(ClipOut_1);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[medium]);
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_1);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[medium]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);

                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
            }
        };

    }
    return;
    //developing now
}




