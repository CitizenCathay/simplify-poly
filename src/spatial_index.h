#pragma once
#include "polygon.h"
#include <vector>
#include <utility>

struct SegmentRef
{
    int u, v;   // indices of the two endpoints in VertexPool::verts
};

struct UniformGrid
{
    double min_x, min_y, max_x, max_y;
    double cell_w, cell_h;
    int    cols, rows;

    std::vector<std::vector<SegmentRef>> cells;

    void build(const VertexPool& pool, int grid_res = 0);

    void insert_segment(const VertexPool& pool, int u, int v);
    void remove_segment(const VertexPool& pool, int u, int v);

    // returns true if segment (px,py)->(qx,qy) intersects any stored segment
    // skipping segments listed in skip_segs
    bool has_intersection(const VertexPool& pool,
                          double px, double py,
                          double qx, double qy,
                          const std::vector<std::pair<int,int>>& skip_segs) const;

private:
    void segment_cells(double px, double py, double qx, double qy,
                       std::vector<int>& out) const;
    int  cell_index(int col, int row) const { return row * cols + col; }
};