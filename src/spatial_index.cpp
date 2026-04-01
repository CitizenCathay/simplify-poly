#include "spatial_index.h"
#include "intersect.h"
#include <cmath>
#include <algorithm>

void UniformGrid::build(const VertexPool& pool, int grid_res)
{
    min_x = min_y =  1e18;
    max_x = max_y = -1e18;

    for (const auto& v : pool.verts)
    {
        if (!v.active) continue;
        min_x = std::min(min_x, v.x);
        max_x = std::max(max_x, v.x);
        min_y = std::min(min_y, v.y);
        max_y = std::max(max_y, v.y);
    }

    min_x -= 1e-9; min_y -= 1e-9;
    max_x += 1e-9; max_y += 1e-9;

    int n = (int)pool.verts.size();
    if (grid_res <= 0)
        grid_res = std::max(1, (int)std::sqrt((double)n));

    cols = grid_res;
    rows = grid_res;
    cell_w = (max_x - min_x) / cols;
    cell_h = (max_y - min_y) / rows;

    cells.assign(cols * rows, {});

    // Insert every active polygon edge exactly once by walking each ring.
    for (int r = 0; r < pool.num_rings; ++r)
    {
        if (pool.ring_size(r) < 2) continue;

        int start = pool.ring_heads[r];
        int u = start;
        do
        {
            int v = pool.next_of(u);
            if (pool.verts[u].active && pool.verts[v].active)
                insert_segment(pool, u, v);
            u = v;
        } while (u != start);
    }
}

void UniformGrid::segment_cells(double px, double py,
                                 double qx, double qy,
                                 std::vector<int>& out) const
{
    // AABB of segment
    double lx = std::min(px, qx);
    double ly = std::min(py, qy);
    double hx = std::max(px, qx);
    double hy = std::max(py, qy);

    int c0 = std::clamp((int)((lx - min_x) / cell_w), 0, cols - 1);
    int c1 = std::clamp((int)((hx - min_x) / cell_w), 0, cols - 1);
    int r0 = std::clamp((int)((ly - min_y) / cell_h), 0, rows - 1);
    int r1 = std::clamp((int)((hy - min_y) / cell_h), 0, rows - 1);

    out.clear();
    for (int r = r0; r <= r1; ++r)
        for (int c = c0; c <= c1; ++c)
            out.push_back(cell_index(c, r));
}

void UniformGrid::insert_segment(const VertexPool& pool, int u, int v)
{
    std::vector<int> cs;
    segment_cells(pool.verts[u].x, pool.verts[u].y,
                  pool.verts[v].x, pool.verts[v].y, cs);
    for (int ci : cs)
        cells[ci].push_back({u, v});
}

void UniformGrid::remove_segment(const VertexPool& pool, int u, int v)
{
    std::vector<int> cs;
    segment_cells(pool.verts[u].x, pool.verts[u].y,
                  pool.verts[v].x, pool.verts[v].y, cs);
    for (int ci : cs)
    {
        auto& cell = cells[ci];
        cell.erase(std::remove_if(cell.begin(), cell.end(),
            [u, v](const SegmentRef& s){
                return (s.u == u && s.v == v) || (s.u == v && s.v == u);
            }), cell.end());
    }
}

bool UniformGrid::has_intersection(const VertexPool& pool,
                                    double px, double py,
                                    double qx, double qy,
                                    const std::vector<std::pair<int,int>>& skip_segs) const
{
    std::vector<int> cs;
    // cast away const for segment_cells (it's logically const)
    const_cast<UniformGrid*>(this)->segment_cells(px, py, qx, qy, cs);

    for (int ci : cs)
    {
        for (auto& seg : cells[ci])
        {
            // skip segments involved in this collapse
            bool skip = false;
            for (auto& [su, sv] : skip_segs)
            {
                if ((seg.u == su && seg.v == sv) || (seg.u == sv && seg.v == su))
                { skip = true; break; }
            }
            if (skip) continue;

            const Vertex& s = pool.verts[seg.u];
            const Vertex& t = pool.verts[seg.v];
            if (segments_intersect(px, py, qx, qy, s.x, s.y, t.x, t.y))
                return true;
        }
    }
    return false;
}