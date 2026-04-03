/* Start Header *****************************************************************/
/*!
\file       polygon.h
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Declares the vertex and vertex-pool data structures used to represent
    polygons with multiple rings.

    This header defines the Vertex structure, which stores the geometric
    coordinates and linked-list connectivity for a polygon vertex, and the
    VertexPool structure, which stores all vertices across all rings along
    with ring metadata such as ring heads, ring sizes, and the total number
    of rings.

    The VertexPool interface provides functionality for loading polygon data
    from CSV, writing the current polygon back to CSV, traversing ring
    connectivity, querying ring sizes, and computing signed polygon areas.
*/
/* End Header *******************************************************************/

#pragma once
#include <vector>
#include <string>

/*!
\struct Vertex
\brief
    Stores a single polygon vertex and its ring connectivity.

\details
    A Vertex represents one point in a polygon ring. In addition to its
    geometric coordinates, it stores indices to the previous and next
    vertices within the same ring, allowing each ring to be represented
    as a circular doubly linked list inside the vertex pool.

    The structure also stores the ring identifier and whether the vertex
    is currently active in the simplified polygon.
*/
struct Vertex
{
    double x, y;      //!< Cartesian coordinates of the vertex.
    int prev, next;   //!< Indices of the previous and next vertices in the same ring.
    int ring_id;      //!< The id of the ring to which this vertex belongs.
    bool active;      //!< Indicates whether this vertex is currently active.
};

/*!
\struct VertexPool
\brief
    Stores all vertices and ring metadata for a polygon with holes.

\details
    The VertexPool structure owns the complete set of polygon vertices and
    organizes them into rings using circular doubly linked connectivity.
    Each ring has an entry point stored in `ring_heads`, and its current
    active vertex count is stored in `ring_sizes`.

    The interface supports loading and writing the project CSV format,
    navigating ring neighbors, querying ring sizes, and computing signed
    areas for individual rings or the entire polygon.
*/
struct VertexPool
{
    std::vector<Vertex> verts;
    std::vector<int>    ring_heads;  //!< ring_heads[r] is the index of the first vertex in ring r.
    std::vector<int>    ring_sizes;  //!< ring_sizes[r] is the current active vertex count in ring r.
    int                 num_rings = 0; //!< Total number of rings stored in the pool.

    /*!
    \brief
        Loads polygon data from a CSV file.

    \details
        Reads the project CSV format, creates all vertices, and builds
        the ring connectivity structure for each ring in the polygon.

    \param[in] path
        The path to the input CSV file.
    */
    void load_csv(const std::string& path);

    /*!
    \brief
        Writes the current polygon data to standard output in CSV format.

    \details
        Outputs the current active polygon state using the required project
        CSV format, including the header and contiguous vertex numbering
        within each ring.
    */
    void write_csv() const;

    /*!
    \brief
        Returns the index of the next vertex in the same ring.

    \param[in] i
        The index of the current vertex.

    \return
        Returns the index of the next vertex in the ring.
    */
    int next_of(int i) const { return verts[i].next; }

    /*!
    \brief
        Returns the index of the previous vertex in the same ring.

    \param[in] i
        The index of the current vertex.

    \return
        Returns the index of the previous vertex in the ring.
    */
    int prev_of(int i) const { return verts[i].prev; }

    /*!
    \brief
        Returns the number of active vertices in a ring.

    \param[in] ring_id
        The id of the ring to query.

    \return
        Returns the current active vertex count of the specified ring.
    */
    int ring_size(int ring_id) const;

    /*!
    \brief
        Computes the signed area of a ring.

    \details
        Uses the shoelace formula to compute the signed area of the
        specified ring. Counterclockwise rings produce positive area and
        clockwise rings produce negative area.

    \param[in] ring_id
        The id of the ring whose area is to be computed.

    \return
        Returns the signed area of the specified ring.
    */
    double signed_area(int ring_id) const;

    /*!
    \brief
        Computes the total signed area of the polygon.

    \details
        Sums the signed areas of all rings in the pool. With the project’s
        orientation convention, the exterior ring contributes positive area
        and holes contribute negative area.

    \return
        Returns the total signed area of the polygon.
    */
    double total_signed_area() const;
};