#include "moon.h"

#if 0
#include <emmintrin.h> // ����SSE2ָ�ͷ�ļ�
//ʹ��SSE2ָ��Ŀ���ƽ����
double SSE2Qsqrt(double number) {
    __m128d vec;
    vec = _mm_load_sd(&number); // ����doubleֵ��SSE�Ĵ���
    vec = _mm_sqrt_pd(vec);   // ����ƽ����
    _mm_store_sd(&number, vec); // ������洢��double����
    return number;
}
#endif

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



double vectorMath::GetLength(const vertix& vec) {
    return KQsqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
};
double vectorMath::GetLength(const double vec[3]) {
    return KQsqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
};
double vectorMath::dot(const vertix& vec1, const vertix& vec2) {
    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
};
void vectorMath::cross(const vertix& vec1, const vertix& vec2, vertix &vec_cross) {
    vec_cross = { (vec2.y * vec1.z - vec2.z * vec1.y), (vec2.z * vec1.x - vec2.x * vec1.z), (vec2.x * vec1.y - vec2.y * vec1.x) };
};