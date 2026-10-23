#include "tgaimage.h"
#include"model.h"
#include <math.h>
#include <ctime>
using namespace std;

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255}; // red yellow blue alpha
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};


void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
    }
}

void lineTest() {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 7,  ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, framebuffer, blue);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, yellow);
    line(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
}


void lineMulTest(){
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    std::srand(std::time({}));
    for (int i=0; i<(1<<24); i++) {
        int ax = rand()%width, ay = rand()%height;
        int bx = rand()%width, by = rand()%height;
        line(ax, ay, bx, by, framebuffer, { rand()%255, rand()%255, rand()%255, rand()%255 });
    }
    framebuffer.write_tga_file("framebufferLine.tga");
}

// hw1 
// 视口变化 丢弃z
std::tuple<int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

void homework1(const string& filename){
    constexpr int width  = 800;
    constexpr int height = 800;
    Model model(filename);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    // 画出来点
    for (int i=0; i<model.nverts(); i++) { // iterate through all vertices
        vec3 v = model.vert(i);            // get i-th vertex
        auto [x, y] = project(v);          // project it to the screen
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("homework1.tga");
}


// sort the vertexs
inline void vertexSort(int& ax, int& ay, int& bx, int& by, int& cx, int& cy){
    if(ay > by){
        swap(ax, bx);
        swap(ay, by);
    }
    if(ay > cy){
        swap(ax, cx);
        swap(ay, cy);
    }
    if(by > cy){
        swap(bx, cx);
        swap(by, cy);
    }
}

// 扫描方法
void triangleScan(int ax, int ay, int bx, int by, int cx, int cy, TGAImage & framebuffer, TGAColor color){
    // 排序
    vertexSort(ax, ay, bx, by, cx, cy);
    
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);

    // 横向线段填充
    int total_height = cy - ay;

    if(ay != by){
        int segment_height = by - ay;
        for(int y = ay; y<=by ; y++){
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            for(int x = min(x1, x2); x < max(x1, x2); x++){
                framebuffer.set(x, y, color);
            }
        }
    }

    if (by != cy) { // if the upper half is not degenerate
        int segment_height = cy - by;
        for (int y=by; y<=cy; y++) { // sweep the horizontal line from by to cy
            int x1 = ax + ((cx - ax)*(y - ay)) / total_height;
            int x2 = bx + ((cx - bx)*(y - by)) / segment_height;
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);
        }
    }

    return ;
}
// 计算任意三角形面积
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

// 没有解决深度问题 包围盒
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            framebuffer.set(x, y, color);
        }
    }
}

// 背部剔除 通过三角形的有效面积正负
// 用z来表示灰度
void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            framebuffer.set(x, y, {z});
        }
    }
}

// 颜色插值
void traingle(int ax, int ay, TGAColor colorA, int bx, int by, TGAColor colorB, int cx, int cy, TGAColor colorC, TGAImage& framebuffer){
    int bbminx = min(ax, min(bx , cx));
    int bbminy = min(ay, min(by, cy));
    int bbmaxx = max(ax, max(bx, cx));
    int bbmaxy = max(ay, max(by, cy));

    double total_area = 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx)) ;
    if(total_area < 1)return;

#pragma omp parallel for
    for(int x = bbminx; x<=bbmaxx; x++){
        for(int y = bbminy; y<=bbmaxy; y++){
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if(alpha < 0 || beta < 0 || gamma <0) continue;

            TGAColor color = colorA.operator*(alpha) + colorB.operator*(beta) + colorC.operator*(gamma);
            framebuffer.set(x, y, color);
        }
    }

}

// 此时规定屏幕指向人的方向是z的正方向
// 所以 z越大越近
void triangleZBuffer(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz,TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            // 与对应位置的灰度值比较
            if(z <= zbuffer.get(x, y)[0]) continue;
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

// 保留深度信息
// 将坐标[-1,1] 映射到 [0, 1] 后再扩大到[0, width]
tuple <int, int, int> projectWithZ(vec3 v){
        return { (v.x + 1.) *  width/2,       // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2,       // we want to shift the vector (x,y) and then scale it to span the entire screen.
             (v.z + 1.) *   255./2 };
}

void triangleTestOriginal(){
    constexpr int width  = 128;
    constexpr int height = 128;

    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangleScan(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangleScan(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangleScan(115, 83, 80,  90, 85, 120, framebuffer, green);

    framebuffer.write_tga_file("triangleTestOriginal.tga");
}


void triangleTestWithGray(){
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::GRAYSCALE);

    int ax = 17, ay =  4, az =  13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    triangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);

    framebuffer.write_tga_file("framebufferWithCGray.tga");
}

// homework3
void triangleTestWithColor(){
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 17, ay =  4, az =  13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    traingle(ax, ay, blue, bx, by, green, cx ,cy, red, framebuffer);

    framebuffer.write_tga_file("framebufferWithColor.tga");
}

void homework2(const string& filename){
    constexpr int width  = 800;
    constexpr int height = 800;
    Model model(filename);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
    }

    framebuffer.write_tga_file("homework2-drwaer-alogrithm.tga");
}

// 
void testZbuffer(){
    constexpr int width  = 800;
    constexpr int height = 800;
    Model model("./diablo3_pose.obj");
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = projectWithZ(model.vert(i, 0));
        auto [bx, by, bz] = projectWithZ(model.vert(i, 1));
        auto [cx, cy, cz] = projectWithZ(model.vert(i, 2));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangleZBuffer(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("ZbufferResColor.tga");
    zbuffer.write_tga_file("ZbufferRes.tga");
}

// 相机操作 模型矩阵
vec3 rot(vec3 v){
    constexpr double a = M_PI / 6 ;
    mat<3,3> Ry= {
            cos(a),     0,  sin(a),
            0,          1,  0,
            -sin(a),    0,  cos(a)    
    };
    return Ry * v ;
}

void TestRotate(){
    constexpr int width  = 800;
    constexpr int height = 800;
    Model model("african_head.obj");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = projectWithZ(rot(model.vert(i, 0)));
        auto [bx, by, bz] = projectWithZ(rot(model.vert(i, 1)));
        auto [cx, cy, cz] = projectWithZ(rot(model.vert(i, 2)));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangleZBuffer(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebufferRotate.tga");
    zbuffer.write_tga_file("zbufferRotate.tga");
}

vec3 perspectiveProject(vec3 v) {
    constexpr double c = 3.;
    return v / (1-v.z/c);
}

void testPerspective(){
    constexpr int width  = 800;
    constexpr int height = 800;
    Model model("african_head.obj");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = projectWithZ(perspectiveProject((rot(model.vert(i, 0)))));
        auto [bx, by, bz] = projectWithZ(perspectiveProject(rot(model.vert(i, 1))));
        auto [cx, cy, cz] = projectWithZ(perspectiveProject(rot(model.vert(i, 2))));
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        triangleZBuffer(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebufferRotatetestPerspective.tga");
    zbuffer.write_tga_file("zbufferRotatetestPerspective.tga");
}


int main(int argc, char** argv) {
    testPerspective();
    return 0;
}

