/* Start Header *****************************************************************/
/*!
\file       spatial_index.h
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Declares the uniform-grid spatial index used for segment intersection
    queries during polygon simplification.

    This header defines the SegmentRef structure, which stores references
    to polygon edges by endpoint indices, and the UniformGrid structure,
    which partitions the polygon’s bounding box into a regular grid of
    cells containing segment references.

    The spatial index supports building the grid from the current polygon,
    inserting and removing segments as the polygon changes, and querying
    whether a proposed segment intersects any stored segment other than
    those explicitly skipped.
*/
/* End Header *******************************************************************/

#pragma once
#include "polygon.h"
#include <vector>
#include <utility>

/*!
\struct SegmentRef
\brief
    Stores a reference to a polygon edge.

\details
    A SegmentRef identifies a segment by storing the indices of its two
    endpoint vertices in the vertex pool. It is used by the uniform grid
    to associate polygon edges with grid cells.
*/
struct SegmentRef
{
    int u, v;   //!< Indices of the two endpoint vertices in VertexPool::verts.
};

/*!
\struct UniformGrid
\brief
    A uniform-grid spatial index for polygon edge intersection queries.

\details
    The UniformGrid partitions the polygon’s bounding box into a regular
    grid of cells. Each cell stores references to segments whose bounding
    boxes overlap that cell. This allows topology checks to restrict
    precise intersection tests to nearby segments rather than scanning all
    polygon edges.

    The grid supports initial construction from a VertexPool, incremental
    insertion and removal of segments as simplification progresses, and
    querying whether a candidate segment intersects any stored segment
    other than an explicitly provided skip set.
*/
struct UniformGrid
{
    double min_x, min_y, max_x, max_y; //!< Bounding box of the indexed geometry.
    double cell_w, cell_h;             //!< Width and height of each grid cell.
    int    cols, rows;                 //!< Number of columns and rows in the grid.

    std::vector<std::vector<SegmentRef>> cells; //!< Segment lists stored per grid cell.

    /*!
    \brief
        Builds the spatial index from the current polygon.

    \details
        Computes the bounding box of all active vertices, initializes the
        grid dimensions, clears existing cell contents, and inserts every
        active polygon edge into the appropriate cells.

    \param[in] pool
        The vertex pool containing the polygon geometry.
    \param[in] grid_res
        The desired grid resolution in both dimensions. If this value is
        zero or negative, a default resolution is chosen automatically.
    */
    void build(const VertexPool& pool, int grid_res = 0);

    /*!
    \brief
        Inserts a segment into the spatial index.

    \param[in] pool
        The vertex pool containing the segment endpoints.
    \param[in] u
        The index of the first endpoint vertex.
    \param[in] v
        The index of the second endpoint vertex.
    */
    void insert_segment(const VertexPool& pool, int u, int v);

    /*!
    \brief
        Removes a segment from the spatial index.

    \param[in] pool
        The vertex pool containing the segment endpoints.
    \param[in] u
        The index of the first endpoint vertex.
    \param[in] v
        The index of the second endpoint vertex.
    */
    void remove_segment(const VertexPool& pool, int u, int v);

    /*!
    \brief
        Tests whether a query segment intersects any stored segment.

    \details
        This function checks whether the segment from
        (px, py) to (qx, qy) intersects any segment currently stored in
        the grid, excluding those listed in \p skip_segs. It is used during
        topology validation when deciding whether a proposed collapse would
        introduce invalid edge crossings.

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
        The list of segments to ignore during the query. Each skipped
        segment is represented as a pair of endpoint indices.

    \return
        Returns true if an intersection is found and false otherwise.
    */
    bool has_intersection(const VertexPool& pool,
                          double px, double py,
                          double qx, double qy,
                          const std::vector<std::pair<int,int>>& skip_segs) const;

private:
    /*!
    \brief
        Computes the grid cells overlapped by a segment’s bounding box.

    \details
        Determines the set of grid cell indices touched by the axis-aligned
        bounding box of the segment from (px, py) to (qx, qy).

    \param[in] px
        The x coordinate of the first endpoint.
    \param[in] py
        The y coordinate of the first endpoint.
    \param[in] qx
        The x coordinate of the second endpoint.
    \param[in] qy
        The y coordinate of the second endpoint.
    \param[out] out
        A vector that receives the indices of overlapping cells.
    */
    void segment_cells(double px, double py, double qx, double qy,
                       std::vector<int>& out) const;

    /*!
    \brief
        Computes the linear index of a grid cell.

    \param[in] col
        The column index of the cell.
    \param[in] row
        The row index of the cell.

    \return
        Returns the flattened array index corresponding to the given cell.
    */
    int cell_index(int col, int row) const { return row * cols + col; }
};