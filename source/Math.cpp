#include "moon.h"

//�����֮�����ƽ����
double KQsqrt(double number) {
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

    return 1 / y;
}


double dot(const Vec3& vec1, const Vec3& vec2) {
    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
};

double dot(const Vec3& vec1, const double vec2[3]) {
    return (vec1.x * vec2[0] + vec1.y * vec2[1] + vec1.z * vec2[2]);
};

Vec3 cross(const Vec3& vec1, const Vec3& vec2) {
    return Vec3 ( (vec2.y * vec1.z - vec2.z * vec1.y), (vec2.z * vec1.x - vec2.x * vec1.z), (vec2.x * vec1.y - vec2.y * vec1.x) );
};

double GetLength(const Vec3& vec) {
    return KQsqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
};

double GetLength(const double vec[3]) {
    return KQsqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
};



//new
//p �� k��λ���� ��ת
Vec3 rotate(const Vec3& p, const Vec3& k , const double angle) {
    /*
    double len = GetLength(k);
    if (len <= 1e-8) {
        return p; // ���׳��쳣
    }
    //k_Ϊ��λ����
    Vec3 k_ = k / len;
    double dotValue = dot(p, k_);
    Vec3 j;
    j.x = p.x - dotValue * k_.x;
    j.y = p.y - dotValue * k_.y;
    j.z = p.z - dotValue * k_.z;

    return Vec3 ( k_ * dotValue + j * cos(angle) + cross(j, k_) * sin(angle) );
    */
    return Vec3(p * cos(angle) + cross(k, p) * sin(angle) + k * dot(k, p) * (1 - cos(angle)));
}