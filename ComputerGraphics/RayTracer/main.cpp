////////////////////////////////////////////////////////////////////////////////
// Author: Tyler Fowler (from provided code at UVic csc 305)
// ID: V00752565
// C++ include
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <algorithm>
#include <numeric>

// Utilities for the Assignment
#include "utils.h"

// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"

// Shortcut to avoid Eigen:: everywhere, DO NOT USE IN .h
using namespace Eigen;

////////////////////////////////////////////////////////////////////////////////
// Class to store tree
////////////////////////////////////////////////////////////////////////////////
class AABBTree
{
public:
    class Node
    {
    public:
        AlignedBox3d bbox;
        int parent;   // Index of the parent node (-1 for root)
        int left;     // Index of the left child (-1 for a leaf)
        int right;    // Index of the right child (-1 for a leaf)
        int triangle; // Index of the node triangle (-1 for internal nodes)
    };

    std::vector<Node> nodes;
    int root;

    AABBTree() = default;                           // Default empty constructor
    AABBTree(const MatrixXd &V, const MatrixXi &F); // Build a BVH from an existing mesh
};

////////////////////////////////////////////////////////////////////////////////
// Scene setup, global variables
////////////////////////////////////////////////////////////////////////////////
const std::string data_dir = DATA_DIR;
const std::string filename("raytrace.png");
const std::string mesh_filename(data_dir + "dragon.off");

//Camera settings
const double focal_length = 2;
const double field_of_view = 0.7854; //45 degrees
const bool is_perspective = true;
const Vector3d camera_position(0, 0, 2);

// Triangle Mesh
bool is_aabb = true;
MatrixXd vertices; // n x 3 matrix (n points)
MatrixXi facets;   // m x 3 matrix (m triangles)
AABBTree bvh;

//Material for the object, same material for all objects
const Vector4d obj_ambient_color(0.0, 0.5, 0.0, 0);
const Vector4d obj_diffuse_color(0.5, 0.5, 0.5, 0);
const Vector4d obj_specular_color(0.2, 0.2, 0.2, 0);
const double obj_specular_exponent = 256.0;
const Vector4d obj_reflection_color(0.7, 0.7, 0.7, 0);

// Precomputed (or otherwise) gradient vectors at each grid node
const int grid_size = 20;
std::vector<std::vector<Vector2d>> grid;

//Lights
std::vector<Vector3d> light_positions;
std::vector<Vector4d> light_colors;
//Ambient light
const Vector4d ambient_light(0.2, 0.2, 0.2, 0);

//Fills the different arrays
void setup_scene()
{
    //Loads file
    std::ifstream in(mesh_filename);
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

    //setup tree
    bvh = AABBTree(vertices, facets);

    //Lights
    light_positions.emplace_back(8, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(6, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(4, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(2, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(0, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(-2, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(-4, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);
}

////////////////////////////////////////////////////////////////////////////////
// BVH Code
////////////////////////////////////////////////////////////////////////////////

AlignedBox3d bbox_from_triangle(const Vector3d &a, const Vector3d &b, const Vector3d &c)
{
    AlignedBox3d box;
    box.extend(a);
    box.extend(b);
    box.extend(c);
    return box;
}

struct SortByCentroid {
    const MatrixXd &Centroids;
    int ax;

    SortByCentroid(const MatrixXd &c, int axis): Centroids(c), ax(axis) {}

    bool operator()(int a, int b)
    {
        return (Centroids(a, ax) < Centroids(b, ax));
    }
};

int make_tree( const MatrixXd &V,
               const MatrixXi &F,
               const MatrixXd &C,
               std::vector<int> &facet_order, 
               std::vector<AABBTree::Node> &nodes,
               int parent)
{   
    // Start by creating a new node.
    AABBTree::Node newNode;
    int idx = nodes.size();
    nodes.push_back(newNode);

    // Base case: 1 triangle => leaf node.
    if (facet_order.size() == 1) {
        int t = facet_order[0];
        nodes[idx].bbox = bbox_from_triangle(V.row(F(t, 0)).transpose(),
                                            V.row(F(t, 1)).transpose(),
                                            V.row(F(t, 2)).transpose());
        nodes[idx].parent = parent;
        nodes[idx].left = -1;
        nodes[idx].right = -1;
        nodes[idx].triangle = t;
        return idx;
    }

    // Interior nodes
    // Sort centroids according to long_axis spanned by triangles.
    int long_axis;
    AlignedBox3d centroid_bbox;
    for (int t : facet_order) {
        centroid_bbox.extend(C.row(t).transpose());
    }
    ((Vector3d)(centroid_bbox.max() - centroid_bbox.min())).maxCoeff(&long_axis);
    SortByCentroid order(C, long_axis);
    std::sort(facet_order.begin(), facet_order.end(), order);

    // Split the input set of triangles into two sets S1 and S2.
    int mid = facet_order.size() / 2;
    std::vector<int> s1(facet_order.begin(), facet_order.begin() + mid);
    std::vector<int> s2(facet_order.begin() + mid, facet_order.end());

    // Recursively build subtrees T1 and T2.
    nodes[idx].triangle = -1;
    nodes[idx].parent = parent;
    nodes[idx].left = make_tree(V, F, C, s1, nodes, idx);
    nodes[idx].right = make_tree(V, F, C, s2, nodes, idx);
    
    // Update the box of the current node by merging T1 and T2's boxes.
    nodes[idx].bbox = nodes[nodes[idx].left].bbox
                    .merged(nodes[nodes[idx].right].bbox);

    // Return new root's index.
    return idx;
}

AABBTree::AABBTree(const MatrixXd &V, const MatrixXi &F)
{
    // Compute the centroids of all the triangles in the input mesh
    std::vector<int> ts;
    MatrixXd centroids(F.rows(), V.cols());
    centroids.setZero();
    for (int i = 0; i < F.rows(); ++i)
    {
        for (int k = 0; k < F.cols(); ++k)
        {
            centroids.row(i) += V.row(F(i, k));
        }
        centroids.row(i) /= F.cols();
        ts.push_back(i);
        }
    // TODO
    // Top-down approach.
    // Split each set of primitives into 2 sets of roughly equal size,
    // based on sorting the centroids along one direction or another.
    root = make_tree(V, F, centroids, ts, nodes, -1);
}

////////////////////////////////////////////////////////////////////////////////
// Intersection code
////////////////////////////////////////////////////////////////////////////////

double ray_triangle_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, const Vector3d &a, const Vector3d &b, const Vector3d &c, Vector3d &p, Vector3d &N)
{
    // TODO
    // Compute whether the ray intersects the given triangle.
    // If you have done the parallelogram case, this should be very similar to it.

    // Modified from my previous assigment submission.
    Matrix3d A;
    Vector3d pu =  b - a;
    Vector3d pv = c - a;
    A << -ray_direction, pu, pv;

    Vector3d intersect = A.inverse() * (ray_origin - a);

    if ((A*intersect).isApprox(ray_origin - a) && intersect[0] >= 0
            && intersect[1] >= 0 && intersect[1] <= 1
            && intersect[2] >= 0 && intersect[2] <= 1
            && intersect[1] + intersect[2] <= 1) {
        // Ray intersects the parallelogram. Return t.
        p = (ray_origin + intersect[0] * ray_direction);
        N = pu.cross(pv).normalized();
        return intersect[0];
    }
    // No intersection.
    return -1;
}

bool ray_box_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, const AlignedBox3d &box)
{
    // TODO
    // Compute whether the ray intersects the given box.
    // we are not testing with the real surface here anyway.
    //
    // Algorithm credit: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
    Vector3d div(1 / ray_direction(0), 1 / ray_direction(1), 1 / ray_direction(2));
    Vector3d mins = (box.min() - ray_origin).cwiseProduct(div);
    Vector3d maxs = (box.max() - ray_origin).cwiseProduct(div);

    if (mins(0) > maxs(0)) {
        double tmp = mins(0);
        mins(0) = maxs(0);
        maxs(0) = tmp;
    }
    if (mins(1) > maxs(1)) {
        double tmp = mins(1);
        mins(1) = maxs(1);
        maxs(1) = tmp;
    }
    if (mins(2) > maxs(2)) {
        double tmp = mins(2);
        mins(2) = maxs(2);
        maxs(2) = tmp;
    }

    double min = mins(0) < mins(1) ? mins(0) : mins(1);
    double max = maxs(0) > maxs(1) ? maxs(0) : maxs(1);

    return (mins(0) <= maxs(1)
            && mins(1) <= maxs(0)
            && min <= maxs(2)
            && mins(2) <= max);
}

//Finds the closest intersecting object returns its index
//In case of intersection it writes into p and N (intersection point and normals)
bool find_nearest_object(const Vector3d &ray_origin, const Vector3d &ray_direction, Vector3d &p, Vector3d &N)
{
    // TODO
    // Method (1): Traverse every triangle and return the closest hit.
    // Method (2): Traverse the BVH tree and test the intersection with a
    // triangles at the leaf nodes that intersects the input ray.
    Vector3d tmp_p, tmp_N;
    int closest_index = -1;
    double closest_t = std::numeric_limits<double>::max(); //closest t is "+ infinity"
    
    if (is_aabb) {
        std::vector<AABBTree::Node> stack;
        stack.push_back(bvh.nodes[bvh.root]);

        while (!stack.empty()) {
            AABBTree::Node n = stack.back();
            stack.pop_back();
            if (ray_box_intersection(ray_origin, ray_direction, n.bbox)) 
            {
                if (n.triangle == -1) { // not a triangle, push on children
                    stack.push_back(bvh.nodes[n.left]);
                    stack.push_back(bvh.nodes[n.right]);
                } else { // Node is a triangle, check for intersection.
                    const double t = ray_triangle_intersection(
                                        ray_origin, ray_direction, 
                                        vertices.row(facets(n.triangle, 0)).transpose(), 
                                        vertices.row(facets(n.triangle, 1)).transpose(), 
                                        vertices.row(facets(n.triangle, 2)).transpose(), 
                                        tmp_p, tmp_N);
                    if (t >= 0.0 && t < closest_t) {
                        closest_t = t;
                        closest_index = n.triangle;
                        p = tmp_p;
                        N = tmp_N;
                    }
                }
            }
        }
    } else {
        double closest_t = std::numeric_limits<double>::max(); //closest t is "+ infinity"
        for (int i = 0; i < facets.rows(); ++i)
        {
            //returns t and writes on tmp_p and tmp_N
            const double t = ray_triangle_intersection(ray_origin, ray_direction,
                                                        vertices.row(facets(i, 0)), 
                                                        vertices.row(facets(i, 1)), 
                                                        vertices.row(facets(i, 2)),
                                                        tmp_p, tmp_N);
            //We have intersection
            if (t >= 0)
            {
                //The point is before our current closest t
                if (t < closest_t)
                {
                    closest_index = i;
                    closest_t = t;
                    p = tmp_p;
                    N = tmp_N;
                }
            }
        }
    }
    return (closest_index >= 0);
}

////////////////////////////////////////////////////////////////////////////////
// Raytracer code
////////////////////////////////////////////////////////////////////////////////

Vector4d shoot_ray(const Vector3d &ray_origin, const Vector3d &ray_direction)
{
    //Intersection point and normal, these are output of find_nearest_object
    Vector3d p, N;

    bool nearest_object = find_nearest_object(ray_origin, ray_direction, p, N);

    if (!nearest_object)
    {
        // Return a transparent color
        return Vector4d(0, 0, 0, 0);
    }

    // Ambient light contribution
    const Vector4d ambient_color = obj_ambient_color.array() * ambient_light.array();

    // Punctual lights contribution (direct lighting)
    Vector4d lights_color(0, 0, 0, 0);
    for (int i = 0; i < light_positions.size(); ++i)
    {
        const Vector3d &light_position = light_positions[i];
        const Vector4d &light_color = light_colors[i];

        Vector4d diff_color = obj_diffuse_color;

        // TODO: Add shading parameters
        double k_diffuse = 1.0;
        double k_specular = 1.0;
        double k_ambient = 1.0;

        // Diffuse contribution
        const Vector3d Li = (light_position - p).normalized();
        const Vector4d diffuse = diff_color * std::max(Li.dot(N), 0.0);

        // Specular contribution
        const Vector3d Hi = (Li - ray_direction).normalized();
        const Vector4d specular = obj_specular_color * std::pow(std::max(N.dot(Hi), 0.0), obj_specular_exponent);
        // Vector3d specular(0, 0, 0);

        // Attenuate lights according to the squared distance to the lights
        const Vector3d D = light_position - p;
        lights_color += (diffuse + specular).cwiseProduct(light_color) / D.squaredNorm();
    }

    // Rendering equation
    Vector4d C = ambient_color + lights_color;

    //Set alpha to 1
    C(3) = 1;

    return C;
}

////////////////////////////////////////////////////////////////////////////////

void raytrace_scene()
{
    std::cout << "Simple ray tracer." << std::endl;

    int w = 640;
    int h = 480;
    MatrixXd R = MatrixXd::Zero(w, h);
    MatrixXd G = MatrixXd::Zero(w, h);
    MatrixXd B = MatrixXd::Zero(w, h);
    MatrixXd A = MatrixXd::Zero(w, h); // Store the alpha mask

    // The camera always points in the direction -z
    // The sensor grid is at a distance 'focal_length' from the camera center,
    // and covers an viewing angle given by 'field_of_view'.
    double aspect_ratio = double(w) / double(h);
    //TODO
    double image_y = std::tan(field_of_view/2.0) * focal_length;
    double image_x = aspect_ratio * image_y;

    // The pixel grid through which we shoot rays is at a distance 'focal_length'
    const Vector3d image_origin(-image_x, image_y, camera_position[2] - focal_length);
    const Vector3d x_displacement(2.0 / w * image_x, 0, 0);
    const Vector3d y_displacement(0, -2.0 / h * image_y, 0);

    for (unsigned i = 0; i < w; ++i)
    {
        for (unsigned j = 0; j < h; ++j)
        {
            const Vector3d pixel_center = image_origin + (i + 0.5) * x_displacement + (j + 0.5) * y_displacement;

            // Prepare the ray
            Vector3d ray_origin;
            Vector3d ray_direction;

            if (is_perspective)
            {
                // Perspective camera
                ray_origin = camera_position;
                ray_direction = (pixel_center - camera_position).normalized();
            }
            else
            {
                // Orthographic camera
                ray_origin = pixel_center;
                ray_direction = Vector3d(0, 0, -1);
            }

            const Vector4d C = shoot_ray(ray_origin, ray_direction);
            R(i, j) = C(0);
            G(i, j) = C(1);
            B(i, j) = C(2);
            A(i, j) = C(3);
        }
    }

    // Save to png
    write_matrix_to_png(R, G, B, A, filename);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    setup_scene();

    raytrace_scene();
    return 0;
}
