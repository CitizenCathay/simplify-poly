/* Start Header *****************************************************************/
/*!
\file       spatial_index.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Cerrero Nicole Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements the uniform-grid spatial index used for segment intersection
    queries during polygon simplification.

    This source file builds and maintains a grid-based spatial partition of
    polygon edges so that topology checks can avoid scanning every segment
    in the polygon. It supports inserting and removing segments as the
    polygon changes, enumerating the grid cells overlapped by a segment,
    and testing whether a proposed new edge intersects any existing edge
    other than explicitly skipped ones.

    The uniform grid improves the efficiency of intersection checking by
    limiting geometric tests to segments stored in relevant nearby cells.
*/
/* End Header *******************************************************************/

#include "spatial_index.h"
#include "intersect.h"
#include <cmath>
#include <algorithm>

/*!
\brief
    Builds the uniform grid from the current polygon state.

\details
    This function computes the bounding box of all active vertices,
    initializes the grid dimensions and cell sizes, clears any existing
    grid contents, and inserts every active polygon edge exactly once by
    traversing each ring.

    If the requested grid resolution is non-positive, the function chooses
    a default resolution based on the square root of the number of stored
    vertices.

\param[in] pool
    The vertex pool containing the polygon geometry.
\param[in] grid_res
    The desired grid resolution in both dimensions. If this value is not
    positive, a default resolution is chosen automatically.
*/
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

/*!
\brief
    Computes the grid cells overlapped by a segment’s axis-aligned bounding box.

\details
    This function determines the bounding box of the segment P -> Q,
    converts that box into grid cell coordinates, clamps them to the grid
    bounds, and outputs the indices of all cells covered by that box.

    The result is used as a coarse spatial filter for insertion,
    removal, and intersection testing.

\param[in] px
    The x coordinate of the first segment endpoint.
\param[in] py
    The y coordinate of the first segment endpoint.
\param[in] qx
    The x coordinate of the second segment endpoint.
\param[in] qy
    The y coordinate of the second segment endpoint.
\param[out] out
    A vector that is cleared and filled with the indices of overlapped
    grid cells.
*/
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

/*!
\brief
    Inserts a segment into all overlapping grid cells.

\details
    This function determines which grid cells are overlapped by the
    segment connecting vertices \p u and \p v and appends a segment
    reference for that edge into each corresponding cell.

\param[in] pool
    The vertex pool containing the segment endpoints.
\param[in] u
    The index of the first endpoint vertex.
\param[in] v
    The index of the second endpoint vertex.
*/
void UniformGrid::insert_segment(const VertexPool& pool, int u, int v)
{
    std::vector<int> cs;
    segment_cells(pool.verts[u].x, pool.verts[u].y,
                  pool.verts[v].x, pool.verts[v].y, cs);
    for (int ci : cs)
        cells[ci].push_back({u, v});
}

/*!
\brief
    Removes a segment from all overlapping grid cells.

\details
    This function finds the grid cells overlapped by the segment
    connecting vertices \p u and \p v and removes any matching segment
    references from those cells. The comparison is performed without
    regard to endpoint order.

\param[in] pool
    The vertex pool containing the segment endpoints.
\param[in] u
    The index of the first endpoint vertex.
\param[in] v
    The index of the second endpoint vertex.
*/
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

/*!
\brief
    Tests whether a segment intersects any non-skipped segment in the grid.

\details
    This function computes the grid cells overlapped by the query segment
    P -> Q, iterates through all stored segment references in those cells,
    ignores any segments listed in the skip set, and performs precise
    intersection tests against the remaining segments.

    It is used during topology validation to determine whether a proposed
    new edge would intersect any existing polygon edge.

\param[in] pool
    The vertex pool containing the stored segment endpoints.
\param[in] px
    The x coordinate of the first query endpoint.
\param[in] py
    The y coordinate of the first query endpoint.
\param[in] qx
    The x coordinate of the second query endpoint.
\param[in] qy
    The y coordinate of the second query endpoint.
\param[in] skip_segs
    A list of segments that should be ignored during the query. Each
    segment is represented as a pair of endpoint indices.

\return
    Returns true if an intersection is found and false otherwise.
*/
bool UniformGrid::has_intersection(const VertexPool& pool,
                                   double px, double py,
                                   double qx, double qy,
                                   const std::vector<std::pair<int,int>>& skip_segs) const
{
    std::vector<int> cs;
    const_cast<UniformGrid*>(this)->segment_cells(px, py, qx, qy, cs);

    for (int ci : cs)
    {
        for (auto& seg : cells[ci])
        {
            bool skip = false;
            for (auto& [su, sv] : skip_segs)
            {
                if ((seg.u == su && seg.v == sv) || (seg.u == sv && seg.v == su))
                {
                    skip = true;
                    break;
                }
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