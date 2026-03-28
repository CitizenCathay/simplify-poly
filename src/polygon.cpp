#include "polygon.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iostream>

// ── CSV parser ────────────────────────────────────────────────────────────────

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

// ── traversal ─────────────────────────────────────────────────────────────────

int VertexPool::ring_size(int ring_id) const
{
    return ring_sizes[ring_id];
}

// ── geometry ──────────────────────────────────────────────────────────────────

// shoelace formula — positive for CCW, negative for CW
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

double VertexPool::total_signed_area() const
{
    double total = 0.0;
    for (int r = 0; r < num_rings; ++r)
        total += signed_area(r);
    return total;
}

// ── output ────────────────────────────────────────────────────────────────────

void VertexPool::write_csv() const
{
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