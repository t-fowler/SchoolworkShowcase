// C++ include
#include <iostream>
#include <string>
#include <vector>

// Utilities for the Assignment
#include "raster.h"

#include <gif.h>
#include <fstream>

#include <Eigen/Geometry>
// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"

using namespace std;
using namespace Eigen;

//Image height
const int H = 480;

//Camera settings
const double near_plane = 1.5; //AKA focal length
const double far_plane = near_plane * 100;
const double field_of_view = 0.7854; //45 degrees
const double aspect_ratio = 1.5;
const bool is_perspective = false;
const Vector3d camera_position(0, 0, 3);
const Vector3d camera_gaze(0, 0, -1);
const Vector3d camera_top(0, 1, 0);

//Object
const std::string data_dir = DATA_DIR;
const std::string mesh_filename(data_dir + "bunny.off");
MatrixXd vertices; // n x 3 matrix (n points)
MatrixXi facets;   // m x 3 matrix (m triangles)

//Material for the object
const Vector3d obj_diffuse_color(0.5, 0.5, 0.5);
const Vector3d obj_specular_color(0.2, 0.2, 0.2);
const double obj_specular_exponent = 256.0;

//Lights
std::vector<Vector3d> light_positions;
std::vector<Vector3d> light_colors;
//Ambient light
const Vector3d ambient_light(0.3, 0.3, 0.3);

//Fills the different arrays
void setup_scene()
{
    //Loads file
    std::ifstream in(mesh_filename);
    if (!in.good())
    {
        std::cerr << "Invalid file " << mesh_filename << std::endl;
        exit(1);
    }
    std::string token;
    in >> token;
    int nv, nf, ne;
    in >> nv >> nf >> ne;
    vertices.resize(nv, 3);
    facets.resize(nf, 3);
    for (int i = 0; i < nv; ++i)
    {
        in >> vertices(i, 0) >> vertices(i, 1) >> vertices(i, 2);
    }
    for (int i = 0; i < nf; ++i)
    {
        int s;
        in >> s >> facets(i, 0) >> facets(i, 1) >> facets(i, 2);
        assert(s == 3);
    }

    //Lights
    light_positions.emplace_back(8, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(6, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(4, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(2, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(0, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(-2, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(-4, 8, 0);
    light_colors.emplace_back(16, 16, 16);
}

Matrix4d compute_rotation(const double alpha)
{
    //TODO: Compute the rotation matrix of angle alpha on the y axis around the object barycenter
    Matrix4d res;
    res << std::cos(alpha), 0, std::sin(alpha), 0,
           0, 1, 0, 0,
           -std::sin(alpha), 0, std::cos(alpha), 0,
           0, 0, 0, 1;
    return res;
}

void build_uniform(UniformAttributes &uniform)
{
    //TODO: setup uniform
    //TODO: setup camera, compute w, u, v
    Vector3d w = -camera_gaze.normalized();
    Vector3d u = camera_top.cross(w).normalized();
    Vector3d v = w.cross(u).normalized();

    //TODO: compute the camera transformation
    Matrix4f C;
    Matrix4f camera_view;
    camera_view << u(0), u(1), u(2), 0,
                   v(0), v(1), v(2), 0,
                   w(0), w(1), w(2), 0,
                   0, 0, 0, 1;
    Matrix4f camera_translate;
    camera_translate << 1, 0, 0, -camera_position(0),
                        0, 1, 0, -camera_position(1),
                        0, 0, 1, -camera_position(2),
                        0, 0, 0, 1;

    C = camera_view * camera_translate;

    //TODO: setup projection matrix
    double t = std::tan(field_of_view/2.0) * near_plane;
    double r = aspect_ratio * t;
    double b = -t;
    double l = -r;
    double n = -near_plane;
    double f = -far_plane;

    Matrix4f P;
    //std::cout << "l: " << l << " r: " << r << " t: " << t << " b: " << b << " n: " << n << " f: " << f << std::endl;
    if (is_perspective)
    {
        //TODO setup prespective camera
        P << 2*n / (r - l),     0,              (l+r) / (l-r),      0,
             0,                 2*n / (t - b),  (b+t) / (b-t),      0,
             0,                 0,              -(f+n) / (n-f),     -(2*f*n) / (f-n),
             0,                 0,              1,                  0;
    }
    else
    {
        P << 2 / (r - l),   0,              0,              -(r + l) / (r - l),
             0,             2 / (t - b),    0,              -(t + b) / (t - b),
             0,             0,              2 / (n - f),    -(n + f) / (n - f),
             0,             0,              0,              1;
    }
    uniform.view = P * C;
    //std::cout << uniform.view << std::endl << std::endl;
}

void simple_render(Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;

    program.VertexShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: fill the shader
        VertexAttributes out;
        out.position = uniform.view * va.position;
        return out;
        //return va;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: fill the shader
        return FragmentAttributes(1, 0, 0);
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        //TODO: fill the shader
        return FrameBufferAttributes(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
    };

    std::vector<VertexAttributes> vertex_attributes;
    //TODO: build the vertex attributes from vertices and facets
    for (int i = 0; i < facets.rows(); i++) {
        MatrixXd a(vertices.row(facets(i, 0))); vertex_attributes.push_back(VertexAttributes(a(0), a(1), a(2)));
        MatrixXd b(vertices.row(facets(i, 1))); vertex_attributes.push_back(VertexAttributes(b(0), b(1), b(2)));
        MatrixXd c(vertices.row(facets(i, 2))); vertex_attributes.push_back(VertexAttributes(c(0), c(1), c(2)));
    }

    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

void wireframe_render(const double alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;

    Matrix4d trafo = compute_rotation(alpha);
    uniform.animation = trafo.cast<float>();

    program.VertexShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: fill the shader
        VertexAttributes out;
        out.position = uniform.view * uniform.animation * va.position;
        return out;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: fill the shader
        return FragmentAttributes(1, 0, 0);
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        //TODO: fill the shader
        return FrameBufferAttributes(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
    };

    std::vector<VertexAttributes> vertex_attributes;

    //TODO: generate the vertex attributes for the edges and rasterize the lines
    for (int i = 0; i < facets.rows(); i++) {
        MatrixXd a(vertices.row(facets(i, 0)));
        MatrixXd b(vertices.row(facets(i, 1)));
        MatrixXd c(vertices.row(facets(i, 2)));

        // line a -> b
        vertex_attributes.push_back(VertexAttributes(a(0), a(1), a(2)));
        vertex_attributes.push_back(VertexAttributes(b(0), b(1), b(2)));

        // line b -> c
        vertex_attributes.push_back(VertexAttributes(b(0), b(1), b(2)));
        vertex_attributes.push_back(VertexAttributes(c(0), c(1), c(2)));

        // line c -> a
        vertex_attributes.push_back(VertexAttributes(c(0), c(1), c(2)));
        vertex_attributes.push_back(VertexAttributes(a(0), a(1), a(2)));
    }
    
    //TODO: use the transformation matrix
    rasterize_lines(program, uniform, vertex_attributes, 0.5, frameBuffer);
}

void get_shading_program(Program &program)
{
    program.VertexShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: transform the position and the normal
        VertexAttributes out;
        out.position = uniform.view * uniform.animation * va.position;
        out.normal = va.normal;

        //TODO: compute the correct lighting
        Vector3d lights_color(0, 0, 0);
        Vector3d p(out.position(0), out.position(1), out.position(2));
        Vector3d N(out.normal(0), out.normal(1), out.normal(2));
        for (int i = 0; i < light_positions.size(); i++) {
            const Vector3d Li = (light_positions[i] - p).normalized();
            const Vector3d diffuse = obj_diffuse_color * std::max(Li.dot(N), 0.0);
            const Vector3d specular = obj_specular_color 
                                    * pow(
                                        std::max(0.0, (Li + (camera_position - p)).normalized().dot(N)), 
                                        obj_specular_exponent
                                        );
            const Vector3d D = light_positions[i] - p;

            lights_color += (diffuse + specular).cwiseProduct(light_colors[i]) / D.squaredNorm();
        }
        lights_color += ambient_light;
        out.colour = Vector4f(lights_color(0), lights_color(1), lights_color(2), 1);
        return out;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        //TODO: create the correct fragment
        FragmentAttributes out(va.colour(0), va.colour(1), va.colour(2), va.colour(3));
        out.position = -va.position;
        return out;
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        //TODO: implement the depth check
        float depth = fa.position(2);

        if (depth < previous.depth) {
            FrameBufferAttributes out(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
            out.depth = depth;
            return out;
        }
        return previous;
    };
}

void flat_shading(const double alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;
    get_shading_program(program);

    Matrix4d trafo = compute_rotation(alpha);
    uniform.animation = trafo.cast<float>();

    //TODO: compute the normals
    //TODO: set material colors
    std::vector<VertexAttributes> vertex_attributes;
    for (int i = 0; i < facets.rows(); i++) {
        // Retrieve vertices from facet i.
        Vector3d a(vertices.row(facets(i, 0)).transpose()); 
        Vector3d b(vertices.row(facets(i, 1)).transpose()); 
        Vector3d c(vertices.row(facets(i, 2)).transpose()); 

        // Calculate the normal.
        Vector3d n = (b - a).cross(c - a).normalized();
        Vector4f face_normal(n(0), n(1), n(2), 0);

        // Add the attributes.
        vertex_attributes.push_back(VertexAttributes(a(0), a(1), a(2)));
        vertex_attributes.back().normal = face_normal;

        vertex_attributes.push_back(VertexAttributes(b(0), b(1), b(2)));
        vertex_attributes.back().normal = face_normal;

        vertex_attributes.push_back(VertexAttributes(c(0), c(1), c(2)));
        vertex_attributes.back().normal = face_normal;
    }

    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

void pv_shading(const double alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;
    get_shading_program(program);

    Matrix4d trafo = compute_rotation(alpha);
    uniform.animation = trafo.cast<float>();

    //TODO: compute the vertex normals as vertex normal average
    //TODO: create vertex attributes
    //TODO: set material colors
    std::vector<VertexAttributes> vertex_attributes;
    std::vector<int> vertex_facets(vertices.rows(), 0); // Initialized to 0
    std::vector<Vector3d> vertex_normals(vertices.rows(), Vector3d(0, 0, 0)); // Initialized to [0, 0, 0]
    for (int i = 0; i < facets.rows(); i++) {
        // Retrieve vertices from facet i.
        Vector3d a(vertices.row(facets(i, 0))); 
        Vector3d b(vertices.row(facets(i, 1))); 
        Vector3d c(vertices.row(facets(i, 2)));

        // Add the facet normal to the vertex normal.
        Vector3d n = (b - a).cross(c - a).normalized();
        Vector3d facet_normal(n(0), n(1), n(2));
        vertex_normals[facets(i, 0)] += facet_normal;
        vertex_normals[facets(i, 1)] += facet_normal;
        vertex_normals[facets(i, 2)] += facet_normal;

        // Track number of facets for each vertex.
        vertex_facets[facets(i, 0)]++;
        vertex_facets[facets(i, 1)]++;
        vertex_facets[facets(i, 2)]++;
    }
    for (int i = 0; i < facets.rows(); i++) {
        // Retrieve vertices from facet i.
        Vector3d a(vertices.row(facets(i, 0))); 
        Vector3d b(vertices.row(facets(i, 1))); 
        Vector3d c(vertices.row(facets(i, 2)));

        // Calculate the average normal for each vertex.
        Vector3d a_norm = (vertex_normals[facets(i, 0)] / vertex_facets[facets(i, 0)]).normalized();
        Vector3d b_norm = (vertex_normals[facets(i, 1)] / vertex_facets[facets(i, 1)]).normalized();
        Vector3d c_norm = (vertex_normals[facets(i, 2)] / vertex_facets[facets(i, 2)]).normalized();

        // Push the attributes.
        vertex_attributes.push_back(VertexAttributes(a(0), a(1), a(2)));
        vertex_attributes.back().normal = Vector4f(a_norm(0), a_norm(1), a_norm(2), 0);

        vertex_attributes.push_back(VertexAttributes(b(0), b(1), b(2)));
        vertex_attributes.back().normal = Vector4f(b_norm(0), b_norm(1), b_norm(2), 0);

        vertex_attributes.push_back(VertexAttributes(c(0), c(1), c(2)));
        vertex_attributes.back().normal = Vector4f(c_norm(0), c_norm(1), c_norm(2), 0);
    }

    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

int main(int argc, char *argv[])
{
    setup_scene();

    int W = H * aspect_ratio;
    Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> frameBuffer(W, H);
    vector<uint8_t> image;
    std::string filename;

    if (is_perspective) filename = "simple_perspective.png";
    else filename = "simple.png";
    simple_render(frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    stbi_write_png(filename.data(), frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    frameBuffer.setConstant(FrameBufferAttributes());

    if (is_perspective) filename = "wireframe_perspective.png";
    else filename = "wireframe.png";
    wireframe_render(0, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    stbi_write_png(filename.data(), frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    frameBuffer.setConstant(FrameBufferAttributes());

    if (is_perspective) filename = "flat_shading_perspective.png";
    else filename = "flat_shading.png";
    flat_shading(0, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    stbi_write_png(filename.data(), frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    frameBuffer.setConstant(FrameBufferAttributes());

    if (is_perspective) filename = "pv_shading_perspective.png";
    else filename = "pv_shading.png";
    pv_shading(0, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    stbi_write_png(filename.data(), frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    frameBuffer.setConstant(FrameBufferAttributes());

    //TODO: add the animation
    const double alpha = std::acos(0.0) / 12.0;
    int delay = 2;
    GifWriter g;

    if (is_perspective) filename = "wireframe_perspective.gif";
    else filename = "wireframe.gif";
    GifBegin(&g, filename.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    for (float i = 0; i < 48; i++)
    {
        frameBuffer.setConstant(FrameBufferAttributes());
        wireframe_render(i * alpha, frameBuffer);
        framebuffer_to_uint8(frameBuffer, image);
        GifWriteFrame(&g, image.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    }
    GifEnd(&g);

    if (is_perspective) filename = "flat_shading_perspective.gif";
    else filename = "flat_shading.gif";
    GifBegin(&g, filename.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    for (float i = 0; i < 48; i++)
    {
        frameBuffer.setConstant(FrameBufferAttributes());
        flat_shading(i * alpha, frameBuffer);
        framebuffer_to_uint8(frameBuffer, image);
        GifWriteFrame(&g, image.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    }
    GifEnd(&g);

    if (is_perspective) filename = "pv_shading_perspective.gif";
    else filename = "pv_shading.gif";
    GifBegin(&g, filename.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    for (float i = 0; i < 48; i++)
    {
        frameBuffer.setConstant(FrameBufferAttributes());
        pv_shading(i * alpha, frameBuffer);
        framebuffer_to_uint8(frameBuffer, image);
        GifWriteFrame(&g, image.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    }
    GifEnd(&g);

    return 0;
}
