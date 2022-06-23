#pragma once

#include <Eigen/Core>

class VertexAttributes
{
public:
    VertexAttributes(float x = 0, float y = 0, float z = 0, float w = 1)
    {
        position << x, y, z, w;
    }

    // Interpolates the vertex attributes
    static VertexAttributes interpolate(
        const VertexAttributes &a,
        const VertexAttributes &b,
        const VertexAttributes &c,
        const float alpha,
        const float beta,
        const float gamma)
    {
        VertexAttributes r;
        r.position = alpha * a.position + beta * b.position + gamma * c.position;
        r.colour = alpha * a.colour + beta * b.colour + gamma * c.colour;
        r.normal = alpha * a.normal + beta * b.normal + gamma * c.normal;
        return r;
    }

    Eigen::Vector4f position;
    Eigen::Vector4f normal;
    Eigen::Vector4f colour;
};

class FragmentAttributes
{
public:
    FragmentAttributes(float r = 0, float g = 0, float b = 0, float a = 1)
    {
        color << r, g, b, a;
    }

    Eigen::Vector4f color;
    Eigen::Vector4f position;
};

class FrameBufferAttributes
{
public:
    FrameBufferAttributes(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255)
    {
        color << r, g, b, a;
        depth = 2;
    }

    Eigen::Matrix<uint8_t, 4, 1> color;
    float depth;
};

class UniformAttributes
{
public:
    Eigen::Matrix4f view;
    Eigen::Matrix4f animation;
};