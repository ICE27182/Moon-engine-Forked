#include "moon.h"

inline Vec3 Transform::To_CameraSpace(const Camera& Receive_camera, const Vec3& Vertex_WorldSpace) {
    Vec3 vec_P = Vertex_WorldSpace - Receive_camera.Pos;
    return Vec3(dot(vec_P, Receive_camera.Forward_vec),
                        dot(vec_P, Receive_camera.Y_vec),
                        dot(vec_P, Receive_camera.Z_vec) );
};

//�������ڽ�ƽ��Ľ�������                          ���                                                           һ����                         ��һ����       
 inline Vec3 Transform::Get_CrossPoint(const Camera& Receive_camera, const Vec3& origin_1, const Vec3& origin_2) {
     double alpha = (Receive_camera.NearPlane - origin_1.x) / (origin_2.x - origin_1.x);
     return Vec3(Receive_camera.NearPlane,
                        (origin_2.y - origin_1.y) * alpha + origin_1.y,
                        (origin_2.z - origin_1.z) * alpha + origin_1.z);

 }

// ������ķ�����                                                  mesh
inline void Transform::Get_NormalVector(Mesh& cMesh, VerticesData& list) {
    Vec3 normal_vectors;
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
        double Length = 1 / GetLength(normal_vector);
        normal_vectors.x = normal_vector[0] * Length;
        normal_vectors.y = normal_vector[1] * Length;
        normal_vectors.z = normal_vector[2] * Length;
        list.norVec.emplace_back(normal_vectors);
    }
    return;
}


inline Vertex2D Transform::CameraSpace_to_ScreenSpace(const Camera& Receive_camera, const long screen_in[2], const Vec3& vertex_CameraSpace) {
    Vec3 VecPlane = vertex_CameraSpace * (Receive_camera.NearPlane / vertex_CameraSpace.x);
    double b = (Receive_camera.F * (screen_in[0] >> 1)) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane);
    return Vertex2D((screen_in[0] >> 1) - (b * VecPlane.y), (screen_in[1] >> 1) - (b * VecPlane.z), vertex_CameraSpace.x, vertex_CameraSpace.y, vertex_CameraSpace.z, 0, 0);//x�洢camera_Space�����
    //ƽ��������ӳ��,����������direcX����ϵ

}

//face��װ+͸��ת��                                               �����                           �ֱ���                                ԭʼmesh                   ���mesh
void Transform::Perspective(const Camera& Receive_camera, const long screen_in[2], Mesh& Original_Mesh, Mesh_R& out) {
    int faceNUM = Original_Mesh.faces.size();
    std::vector <bool> Vertex_info;//��ɼ��Լ��
    Vertex_info.reserve(faceNUM);

    std::vector <bool> CameraSpaceVertex_exist_info;//�����Լ��
    CameraSpaceVertex_exist_info.reserve(faceNUM);

    VerticesData Vertices_data;
    Vertices_data.vertex2d.reserve(Original_Mesh.vertices.size());

    
    for (const Vec3& each_point : Original_Mesh.vertices) {
        Vertex2D Transformed_P;
        if (double depth = dot(each_point - Receive_camera.Pos, Receive_camera.Forward_vec ); depth >= Receive_camera.NearPlane) {
            Vertex_info.push_back(true);
            CameraSpaceVertex_exist_info.push_back(true);
            //ת����camera space
            Vec3 CameraSpace_P = { depth,
                                           dot(each_point - Receive_camera.Pos, Receive_camera.Y_vec),
                                           dot(each_point - Receive_camera.Pos, Receive_camera.Z_vec) };
            
            Transformed_P = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, CameraSpace_P );            //��Ļ����
        } else { 
             //���ɼ����
            Vertex_info.push_back(false);
            CameraSpaceVertex_exist_info.push_back(false);
        }
        Vertices_data.vertex2d.emplace_back(Transformed_P);
    }
    
    if (not std::any_of(Vertex_info.begin(), Vertex_info.end(), [](bool value) { return value; })) return;//ֱ��������ȫ���ɼ���mesh

    Vertices_data.norVec.reserve(faceNUM);
    Get_NormalVector(Original_Mesh, Vertices_data);

    //����Ϊ��λ��ÿ���Ƕ������������ηָ���в��ɼ����ƽ�棨һ���㲻�ɼ�������������㲻�ɼ�������������㶼���ɼ��������
    for (int i = 0; i < faceNUM; ++i) {
        Face* ptrFace = &Original_Mesh.faces[i];
        int index0 = Original_Mesh.faces[i].index[0];

        Vec3 vecTest = Original_Mesh.vertices[index0] - Receive_camera.Pos;
        if (dot(vecTest, Vertices_data.norVec[i]) >= 0) continue;

        int index1 = Original_Mesh.faces[i].index[1];
        int index2 = Original_Mesh.faces[i].index[2];
        //��ȫ�ɼ������
        if (Vertex_info[index0] && Vertex_info[index1] && Vertex_info[index2] ) {

            //����˴�����uv�����Ľ��

            out.vertices.emplace_back(Vertices_data.vertex2d[index0]);
            out.vertices.emplace_back(Vertices_data.vertex2d[index1]);
            out.vertices.emplace_back(Vertices_data.vertex2d[index2]);
            
            out.normal_vectors.emplace_back(Vertices_data.norVec[i]);

            out.color.emplace_back(Original_Mesh.color[i]);
        }
        else {
            //�������洦��(��Ȼ�������������ɼ���)
            int num = static_cast<int>(not Vertex_info[index0]) + static_cast<int>(not Vertex_info[index1]) + static_cast<int>(not Vertex_info[index2]);
            if (num == 3) continue;
            bool num_bool = (num == 1) ? true : false;

            unsigned int previous{}, next{}, medium{};//          previous Vec3, next Vec3, medium Vec3    (��������Է����ǲ��ɼ��㻹�ǿɼ���)(����ֵ)
            Vec3 ClipOut_3d_1, ClipOut_3d_2;            //          3d Vec3 1        3d Vec3 2
            Vertex2D ClipOut_1, ClipOut_2;            //          Vec3 1        Vec3 2
            //Ϊ��������ɵĵ��������Ȼ������˳ʱ���˳��
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Vertex_info[(*ptrFace).index[e]]) {//��ô������            //�˴����������ֱ���1�������2���
                    medium = (*ptrFace).index[e];
                    previous = (*ptrFace).index[(e + 5) % 3];
                    next = (*ptrFace).index[(e + 7) % 3];
                    break;
                }
            }

            if (num_bool && (not CameraSpaceVertex_exist_info[medium]) ) {
                CameraSpaceVertex_exist_info[medium] = true;
                Vec3 tmp = To_CameraSpace(Receive_camera, Original_Mesh.vertices[medium]);
                Vertices_data.put3DIndex(tmp, medium);
            }else{
                if (not CameraSpaceVertex_exist_info[previous]) {
                    CameraSpaceVertex_exist_info[previous] = true;
                    Vec3 tmp = To_CameraSpace(Receive_camera, Original_Mesh.vertices[previous]);
                    Vertices_data.put3DIndex(tmp, previous);

                }if (not CameraSpaceVertex_exist_info[next]) {
                    CameraSpaceVertex_exist_info[next] = true;
                    Vec3 tmp = To_CameraSpace(Receive_camera, Original_Mesh.vertices[next]);
                    Vertices_data.put3DIndex(tmp, next);
                }
            }

            ClipOut_3d_1 = Get_CrossPoint(Receive_camera, Vertices_data.get3D(previous), Vertices_data.get3D(medium));
            ClipOut_3d_2 = Get_CrossPoint(Receive_camera, Vertices_data.get3D(next), Vertices_data.get3D(medium));
            ClipOut_1 = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_1);
            ClipOut_2 = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_2);
            //������դ����������ӳ��ʱ���ǵ������uv����ת����

            if (num_bool) {

                out.vertices.emplace_back(Vertices_data.vertex2d[previous]);
                out.vertices.emplace_back(ClipOut_1);
                out.vertices.emplace_back(ClipOut_2);

                out.normal_vectors.emplace_back(Vertices_data.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
                
                //�˴�����ڶ�����
                out.vertices.emplace_back(ClipOut_2);
                out.vertices.emplace_back(Vertices_data.vertex2d[next]);
                out.vertices.emplace_back(Vertices_data.vertex2d[previous]);

                out.normal_vectors.emplace_back(Vertices_data.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
                
            }
            else {
                //���е�һ��������
                out.vertices.emplace_back(ClipOut_1);
                out.vertices.emplace_back(Vertices_data.vertex2d[medium]);
                out.vertices.emplace_back(ClipOut_2);

                out.normal_vectors.emplace_back(Vertices_data.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
            }
        };

    }
    return;
    //developing now
}




