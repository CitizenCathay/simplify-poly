#pragma once
#include <vector>
#include <string>

struct Vertex
{
    double x, y;
    int prev, next;   // indices into VertexPool::verts
    int ring_id;
    bool active;
};

struct VertexPool
{
    std::vector<Vertex> verts;
    std::vector<int>    ring_heads;  // ring_heads[r] = index of first vertex in ring r
    std::vector<int>    ring_sizes;  // ring_sizes[r] = current active count
    int                 num_rings = 0;

    void   load_csv(const std::string& path);
    void   write_csv() const;

    int    next_of(int i) const { return verts[i].next; }
    int    prev_of(int i) const { return verts[i].prev; }
    int    ring_size(int ring_id) const;

    double signed_area(int ring_id) const;
    double total_signed_area() const;
};