#pragma once
#include <iostream>
#include <cassert>
#include <cmath>
using namespace std;

// 重载向量的vector 的[]运算符
template<int n> 
struct vec{
    double data[n] = {0};
    double& operator[](const int i) {assert(i >= 0 && i<n); return data[i];}
    double operator[](const int i) const {assert(i >= 0 && i<n); return data[i];}
};

// dot operator * 点积
template<int n> 
double operator*(const vec<n>& lhs, const vec<n>& rhs){
    double ret = 0;
    for(int i = n; i--; ret+= lhs[i] * rhs[i]);
    return ret;
}
// vector 加法
template<int n>
vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs){
    vec<n> ret = lhs;
    for(int i =n;i--;ret[i]+=rhs[i]);
    return ret;
}

// 向量减法
template<int n>
vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs){
    vec<n> ret = lhs;
    for(int i =n;i--;ret[i]-=rhs[i]);
    return ret;
}

// 向量乘以标量
template<int n> 
vec<n> operator*(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]*=rhs);
    return ret;
}

// 向量乘以标量重载
template<int n> 
vec<n> operator*(const double& lhs, const vec<n> &rhs) {
    return rhs * lhs;
}

// 除法向量
template<int n> 
vec<n> operator/(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i=n; i--; ret[i]/=rhs);
    return ret;
}

// 重载<< 支持链式调用！
template<int n>
ostream& operator<<(ostream& out,const vec<n>& v){
    for(int i = 0;i<n;i++){
        out << v[i] <<" ";
    }
    return out;
}


// 显示特化
template<>
struct vec<2>{
    double x = 0, y = 0;
    double& operator[](const int i) {assert(i >=0 && i < 2);  return i ? y : x ;}
    double operator[] (const int i) const {assert(i >=0 && i < 2); return i ? y : x ;}
};

template<>
struct vec<3>{
    double x =0, y = 0, z= 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

template<> struct vec<4> {
    double x = 0, y = 0, z = 0, w = 0;
    double& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
    double  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
    vec<2> xy()  const { return {x, y};    }
    vec<3> xyz() const { return {x, y, z}; }
};

// typedef vec<2> vec2;
// typedef vec<3> vec3;
// typedef vec<4> vec4;

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

// 求模
template<int n>
double norm(const vec<n>& v){
    return sqrt(v*v);
}

// 归一化
template<int n>
vec<n> normlize(const vec<n>& v){
    return v / norm(v) ;
}

// 叉积
inline vec3 cross(const vec3& v1, const vec3& v2){
    return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}



template<int n> struct dt;

template<int nrows, int ncols>
struct mat{
    // 数组 有nrows行；每一行有ncol列
    vec<ncols> rows[nrows] = {{}};
    // 访问其中一行
    // 得到对应行的结果是n列 ； vec也重载了因此可以连续[]访问
    vec<ncols>& operator[](const int idx) {assert(idx>=0 && idx<nrows); return rows[idx];}
    const vec<ncols>& operator[](const int idx) const {assert(idx>=0 && idx<nrows); return rows[idx];} 

    // 计算行列式
    double det() const{
        return dt<ncols>::det(*this);
    }

    // 代数余子式
    double cofactor(const int row, const int col) const{
        mat<nrows-1, ncols-1> submatrix;
        for(int i = 0; i < nrows - 1; i++){
            for(int j = 0;j < ncols - 1; j++){
                // 跳过展开点所在的行和列
                submatrix[i][j] = rows[i+ int(i >= row)][j + int(j >= col)];
            }
        }
        return submatrix.det() * ((row + col)%2 ? -1 : 1);
    }

    // 逆转置 A^(-1) 的转置
    mat<nrows, ncols> invert_transpose() const{
        // 伴随矩阵
        mat<nrows, ncols> adjugate_transpose;
        for(int i = 0;i <nrows ;i++){
            for(int j = 0;j< ncols ;j++){
                adjugate_transpose[i][j] = cofactor(i,j);
            }
        }
        // 除以行列式就得到最终结果
        return adjugate_transpose / (adjugate_transpose[0] * rows[0]) ;
    }

    mat<nrows,ncols> invert() const {
        return invert_transpose().transpose();
    }

    // 转置
    mat<ncols,nrows> transpose() const {
        mat<ncols,nrows> ret;
        for (int i=ncols; i--; )
            for (int j=nrows; j--; ret[i][j]=rows[j][i]);
        return ret;
    }

};

// 1 x nrows 向量与 nrows x ncols 矩阵乘法 =>  1 x ncols
template<int nrows,int ncols> 
vec<ncols> operator*(const vec<nrows>& lhs, const mat<nrows,ncols>& rhs) {
    return (mat<1,nrows>{{lhs}}*rhs)[0];
}

// nrows x ncols 与 ncols x 1 
template<int nrows, int ncols>
vec<nrows> operator*(const mat<nrows, ncols>& lhs, const vec<ncols>& rhs){
    vec<nrows> ret;
    for(int i = 0; i< nrows;i++){
        // 两个向量的乘法 每个行向量与列向量乘法
        ret[i] = lhs[i] * rhs ;
    }
    return ret;
}

// 矩阵乘法
template<int R1, int C1, int C2>
mat<R1, C2>operator*(const mat<R1, C1>&lhs, const mat<C1, C2>& rhs){
    mat<R1, C2> ret ;
    for(int i = 0; i < R1 ;i++){
        for(int j = 0; j < C2 ; j++){
            for(int k = 0; k< C1; k++){
                ret[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }
    return ret;
}

// 矩阵乘以数字
template<int nrows,int ncols>
mat<nrows,ncols> operator*(const mat<nrows,ncols>& lhs, const double& val) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; result[i] = lhs[i]*val);
    return result;
}

// 矩阵除以数字
template<int nrows,int ncols>
mat<nrows,ncols> operator/(const mat<nrows,ncols>& lhs, const double& val) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; result[i] = lhs[i]/val);
    return result;
}

// 矩阵加法
template<int nrows,int ncols>
mat<nrows,ncols> operator+(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; )
        for (int j=ncols; j--; result[i][j]=lhs[i][j]+rhs[i][j]);
    return result;
}

// 矩阵减法
template<int nrows,int ncols>
mat<nrows,ncols> operator-(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
    mat<nrows,ncols> result;
    for (int i=nrows; i--; )
        for (int j=ncols; j--; result[i][j]=lhs[i][j]-rhs[i][j]);
    return result;
}

// 输出
template<int nrows, int ncols>
ostream& operator<<(ostream& out, const mat<nrows, ncols>& matrix){
    for(int i = 0; i < nrows ; i++){
        out << matrix[i] << endl;
    }
    return out;
}

// 模板元编程 递归到一个
template<int n>
struct dt{
    static double det(const mat<n,n>& src){
        double ret = 0;
        for(int i = n; i-- ; ret += src[0][i] * src.cofactor(0,i));
        return ret;
    }
};

template<>
struct dt<1>{
    static double det(const mat<1,1>& src) {
        return src[0][0];
    }
};