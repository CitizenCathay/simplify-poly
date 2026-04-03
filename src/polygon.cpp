/* Start Header *****************************************************************/
/*!
\file       polygon.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Cerrero Nicole Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements CSV loading, ring traversal, area computation, and CSV
    output for the polygon vertex pool.

    This source file defines the member functions of the VertexPool class.
    It is responsible for reading polygon data from the project CSV format,
    constructing the circular doubly linked representation for each ring,
    querying ring sizes, computing signed areas using the shoelace formula,
    and writing the current polygon state back to standard output.

    The implementation assumes the input rows are ordered by ring id and
    vertex id as specified by the project format.
*/
/* End Header *******************************************************************/

#include "polygon.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <iomanip>

/*!
\brief
    Loads polygon data from a CSV file into the vertex pool.

\details
    This function opens the specified CSV file, skips the header row,
    reads all vertex records, determines the total number of rings,
    allocates storage for all vertices, and builds a circular doubly
    linked list for each ring.

    The function assumes that the CSV rows are already sorted by ring id
    and then by vertex id, matching the project specification. Each ring
    is stored as a circular sequence so that traversal can move efficiently
    through previous and next vertices.

    If the file cannot be opened or contains no vertex data, the function
    throws a runtime error.

\param[in] path
    The path to the input CSV file.

\exception std::runtime_error
    Thrown if the file cannot be opened or if no vertex data is found.
*/
void VertexPool::load_csv(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    std::string line;
    std::getline(file, line); // skip header

    // first pass: read all rows
    struct Row { int ring_id, vertex_id; double x, y; };
    std::vector<Row> rows;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string tok;
        Row r;
        std::getline(ss, tok, ','); r.ring_id   = std::stoi(tok);
        std::getline(ss, tok, ','); r.vertex_id = std::stoi(tok);
        std::getline(ss, tok, ','); r.x         = std::stod(tok);
        std::getline(ss, tok, ','); r.y         = std::stod(tok);
        rows.push_back(r);
    }

    if (rows.empty())
        throw std::runtime_error("No vertex data found in file");

    // count rings
    num_rings = 0;
    for (auto& r : rows)
        num_rings = std::max(num_rings, r.ring_id + 1);

    ring_heads.assign(num_rings, -1);
    ring_sizes.assign(num_rings, 0);

    // allocate vertices
    verts.resize(rows.size());

    // group rows by ring to build prev/next links
    // rows are assumed sorted by ring_id then vertex_id (as per spec)
    int idx = 0;
    int ring = 0;
    while (ring < num_rings)
    {
        int ring_start = idx;
        while (idx < (int)rows.size() && rows[idx].ring_id == ring)
            ++idx;
        int ring_end = idx; // exclusive
        int count = ring_end - ring_start;

        ring_heads[ring] = ring_start;
        ring_sizes[ring] = count;

        for (int i = ring_start; i < ring_end; ++i)
        {
            auto& row = rows[i];
            auto& v   = verts[i];
            v.x       = row.x;
            v.y       = row.y;
            v.ring_id = ring;
            v.active  = true;
            // circular doubly linked list within ring
            v.prev = (i == ring_start) ? ring_end - 1 : i - 1;
            v.next = (i == ring_end - 1) ? ring_start : i + 1;
        }
        ++ring;
    }
}

/*!
\brief
    Returns the number of active vertices stored in a ring.

\details
    This function retrieves the recorded size of the specified ring from
    the ring size table maintained by the vertex pool.

\param[in] ring_id
    The id of the ring whose size is being queried.

\return
    Returns the number of vertices currently stored in the ring.
*/
int VertexPool::ring_size(int ring_id) const
{
    return ring_sizes[ring_id];
}

/*!
\brief
    Computes the signed area of a ring.

\details
    This function applies the shoelace formula to the vertices of the
    specified ring by traversing its circular linked structure. The result
    is positive for counterclockwise rings and negative for clockwise rings.

    This signed convention is useful because the exterior ring is expected
    to contribute positive area, while interior rings contribute negative
    area.

\param[in] ring_id
    The id of the ring whose signed area is to be computed.

\return
    Returns the signed area of the specified ring.
*/
double VertexPool::signed_area(int ring_id) const
{
    double area = 0.0;
    int start = ring_heads[ring_id];
    int i = start;
    do {
        const Vertex& a = verts[i];
        const Vertex& b = verts[verts[i].next];
        area += (a.x * b.y) - (b.x * a.y);
        i = verts[i].next;
    } while (i != start);
    return area * 0.5;
}

/*!
\brief
    Computes the total signed area of the polygon.

\details
    This function sums the signed areas of all rings in the vertex pool.
    With the project’s ring orientation convention, the exterior ring
    contributes positive area and each hole contributes negative area.

\return
    Returns the total signed area across all rings.
*/
double VertexPool::total_signed_area() const
{
    double total = 0.0;
    for (int r = 0; r < num_rings; ++r)
        total += signed_area(r);
    return total;
}

/*!
\brief
    Writes the current polygon data to standard output in CSV format.

\details
    This function prints the required CSV header followed by one row for
    each vertex in every ring. Vertices are output in ring order, and each
    ring is traversed through its circular linked structure. Vertex ids are
    regenerated contiguously during output.

    Coordinates are written with high precision so that the output preserves
    enough detail for floating-point round-tripping.

\return
    This function does not return a value.
*/
void VertexPool::write_csv() const
{
    std::cout << std::setprecision(15);
    std::cout << "ring_id,vertex_id,x,y\n";
    for (int r = 0; r < num_rings; ++r)
    {
        int start = ring_heads[r];
        int i     = start;
        int vid   = 0;
        do {
            const Vertex& v = verts[i];
            // print with enough precision to round-trip doubles
            std::cout << r << ',' << vid << ','
                      << v.x << ',' << v.y << '\n';
            ++vid;
            i = verts[i].next;
        } while (i != start);
    }
}