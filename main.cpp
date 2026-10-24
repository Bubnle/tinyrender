#include "tgaimage.h"
#include"model.h"
#include <math.h>
#include <ctime>
#include <algorithm>
using namespace std;

// 齐次坐标 来表示变换矩阵 mv 就是模型矩阵和视图矩阵
mat<4,4> ModelView, ViewPort, Perspective ;

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

// 齐次坐标

// 视口变化 的矩阵形式是先缩放再平移
// 视口左上角 x , y
// [-1,1] x [-1,1]-> [x, x + w] x [y, y + h]
void viewport(const int x, const int y, const int w, const int h) {
    ViewPort = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

// 透视矩阵
void perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

// View 
// 透视投影 当中假设的是物体在中心！！！
void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normlize(eye-center);
    vec3 l = normlize(cross(up,n));
    vec3 m = normlize(cross(n, l));
    // 先平移到原点
    // 再进行坐标系的变化
    // 先从世界坐标系到相机坐标系 得到M ；的逆就是逆过程
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}


void rasterize(const vec4 clip[3], std::vector<double> &zbuffer, TGAImage &framebuffer, const TGAColor color){
    // normalized device coordinates  NDC 
    // 得到裁剪空间当中的
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };               
    vec2 screen[3] = { (ViewPort*ndc[0]).xy(), (ViewPort*ndc[1]).xy(), (ViewPort*ndc[2]).xy() }; // screen coordinates

    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if(ABC.det() < 1) return ; // 背面剔除  + 面积太小的三角形去掉

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners

    #pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) { // clip the bounding box by the screen
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            // barycentric coordinates of {x,y} w.r.t the triangle 重心坐标
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; 
            // negative barycentric coordinate => the pixel is outside the triangle
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;       
            // 深度插值 得到当前点深度；                                             
            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}


void testRasterize(){
    constexpr int width  = 800;    // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1,0,2}; // camera position
    constexpr vec3 center{0,0,0};  // camera direction
    constexpr vec3     up{0,1,0};  // camera up vector


    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());


    Model model("./african_head.obj");
    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        vec4 clip[3];
        for (int d : {0,1,2}) {            // assemble the primitive
            vec3 v = model.vert(i, d);
            clip[d] = Perspective * ModelView * vec4{v.x, v.y, v.z, 1.};
        }
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
        rasterize(clip, zbuffer, framebuffer, rnd); // rasterize the primitive
    }
    

    framebuffer.write_tga_file("rasteriza.tga");

}


int main(int argc, char** argv) {
    testRasterize();
    return 0;
}

