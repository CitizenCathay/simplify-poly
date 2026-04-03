
/* Start Header *****************************************************************/
/*!
\file       test_simplify.cpp
\author     Alyssa Nicole Cerrero Alejandro, 2402435
\co-author  Chan Qi Ying, 2402302
\co-author  Choi Meng Yew, 2401822
\date       Apr 03, 2026
\brief

*/
/* End Header *******************************************************************/

#include "polygon.h"
#include "simplify.h"
#include "intersect.h"
#include "spatial_index.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <iomanip>

// Checks if a single ring has any self-intersections
// Returns true if ring is valid, false if self-intersection found
bool check_ring_no_self_intersections(const VertexPool& pool, int ring_id) {
    // Rings with less than 4 vertices cannot self-intersect
    if (pool.ring_size(ring_id) < 4) return true;
    
    // Collect all edges in the ring
    std::vector<std::pair<int, int>> edges;
    int start = pool.ring_heads[ring_id];
    int current = start;
    do {
        int next = pool.next_of(current);
        edges.push_back({current, next});
        current = next;
    } while (current != start);
    
    // Check each pair of non-adjacent edges for intersection
    for (size_t i = 0; i < edges.size(); ++i) {
        for (size_t j = i + 2; j < edges.size(); ++j) {
            // Skip adjacent edges (share a vertex)
            if (j == i + 1) continue;
            // Skip the wrap-around pair (first and last edges)
            if (i == 0 && j == edges.size() - 1) continue;
            
            int u1 = edges[i].first;
            int v1 = edges[i].second;
            int u2 = edges[j].first;
            int v2 = edges[j].second;
            
            const Vertex& a = pool.verts[u1];
            const Vertex& b = pool.verts[v1];
            const Vertex& c = pool.verts[u2];
            const Vertex& d = pool.verts[v2];
            
            if (segments_intersect(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y)) {
                std::cout << "Self-intersection in ring " << ring_id << "\n";
                return false;
            }
        }
    }
    return true;
}

// Validates entire polygon topology
// Checks: no self-intersections in any ring, no crossings between rings
// Returns true if topology is valid, false otherwise
bool validate_topology(const VertexPool& pool) {
    std::cout << "Validating topology...\n";
    
    // Check each ring for self-intersections
    for (int r = 0; r < pool.num_rings; ++r) {
        if (!check_ring_no_self_intersections(pool, r)) {
            return false;
        }
    }
    
    // Check for crossings between different rings
    for (int r1 = 0; r1 < pool.num_rings; ++r1) {
        for (int r2 = r1 + 1; r2 < pool.num_rings; ++r2) {
            // Collect all edges from first ring
            std::vector<std::pair<int, int>> edges1;
            int start1 = pool.ring_heads[r1];
            int curr1 = start1;
            do {
                edges1.push_back({curr1, pool.next_of(curr1)});
                curr1 = pool.next_of(curr1);
            } while (curr1 != start1);
            
            // Collect all edges from second ring
            std::vector<std::pair<int, int>> edges2;
            int start2 = pool.ring_heads[r2];
            int curr2 = start2;
            do {
                edges2.push_back({curr2, pool.next_of(curr2)});
                curr2 = pool.next_of(curr2);
            } while (curr2 != start2);
            
            // Check each edge pair between the two rings
            for (auto& e1 : edges1) {
                for (auto& e2 : edges2) {
                    const Vertex& a = pool.verts[e1.first];
                    const Vertex& b = pool.verts[e1.second];
                    const Vertex& c = pool.verts[e2.first];
                    const Vertex& d = pool.verts[e2.second];
                    
                    if (segments_intersect(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y)) {
                        std::cout << "Ring crossing between ring " << r1 << " and " << r2 << "\n";
                        return false;
                    }
                }
            }
        }
    }
    
    std::cout << "Topology valid\n";
    return true;
}

// Main test function
// Compares original polygon state with simplified state
// Verifies three rubric criteria: area preservation, topology, ring count
void run_test(const std::string& name, 
              const std::string& input_file,
              int target_vertices) {
    
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << name << "\n";
    std::cout << "========================================\n";
    std::cout << "Input file: " << input_file << "\n";
    std::cout << "Target vertices: " << target_vertices << "\n";
    
    // Load polygon from CSV file
    VertexPool pool;
    try {
        pool.load_csv(input_file);
    } catch (const std::exception& e) {
        std::cout << "FAIL: Could not load file - " << e.what() << "\n";
        return;
    }
    
    // Record original polygon state
    int original_ring_count = pool.num_rings;
    double original_area = std::abs(pool.total_signed_area());
    
    int original_total_verts = 0;
    for (int r = 0; r < original_ring_count; ++r) {
        original_total_verts += pool.ring_size(r);
    }
    
    std::cout << "\n--- ORIGINAL ---\n";
    std::cout << "Ring count: " << original_ring_count << "\n";
    std::cout << "Total vertices: " << original_total_verts << "\n";
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Area: " << original_area << "\n";
    
    // Run the simplification algorithm
    std::cout << "\nRunning simplification...\n";
    double displacement = simplify(pool, target_vertices);
    
    // Record final polygon state after simplification
    int final_ring_count = pool.num_rings;
    double final_area = std::abs(pool.total_signed_area());
    double area_error = std::abs(original_area - final_area);
    
    int final_total_verts = 0;
    for (int r = 0; r < final_ring_count; ++r) {
        final_total_verts += pool.ring_size(r);
    }
    
    std::cout << "\n--- FINAL ---\n";
    std::cout << "Ring count: " << final_ring_count << "\n";
    std::cout << "Total vertices: " << final_total_verts << "\n";
    std::cout << "Area: " << final_area << "\n";
    std::cout << "Displacement: " << displacement << "\n";
    
    // Verification section - only rubric criteria
    std::cout << "\n--- VERIFICATION ---\n";
    
    // Criterion 1: Area preservation within tolerance
    const double TOLERANCE = 1e-6;
    bool area_ok = (area_error <= TOLERANCE);
    std::cout << "Area error: " << std::scientific << area_error << "\n";
    std::cout << "Area preserved (error <= 1e-6): " << (area_ok ? "PASS" : "FAIL") << "\n";
    
    // Criterion 2: Topology preservation (no self-intersections, no ring crossings)
    bool topology_ok = validate_topology(pool);
    std::cout << "Topology valid: " << (topology_ok ? "PASS" : "FAIL") << "\n";
    
    // Criterion 3: Ring count unchanged
    bool ring_count_ok = (final_ring_count == original_ring_count);
    std::cout << "Ring count unchanged: " << (ring_count_ok ? "PASS" : "FAIL") << "\n";
    
    // Final verdict based only on rubric criteria
    std::cout << "\n--- RESULTS ---\n";
    if (area_ok && topology_ok && ring_count_ok) {
        std::cout << "ALL TESTS PASSED\n";
        std::cout << "   - Area preserved\n";
        std::cout << "   - Topology preserved\n";
        std::cout << "   - Ring count unchanged\n";
    } else {
        std::cout << "RUBRIC TESTS FAILED\n";
    }
}

// Program entry point
// Expects three command line arguments: test name, input file path, target vertex count
int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "Polygon Simplification Test Suite\n";
    std::cout << "========================================\n";
    
    // Check for correct number of arguments
    if (argc == 4) {
        std::string name = argv[1];
        std::string input_file = argv[2];
        int target_vertices = std::stoi(argv[3]);
        
        run_test(name, input_file, target_vertices);
        return 0;
    }
    
    // Display usage information if arguments are incorrect
    std::cout << "\nUsage:\n";
    std::cout << "  ./test \"Test Name\" input.csv target_vertices\n";
    std::cout << "\nExample:\n";
    std::cout << "  ./test \"Dataset 1\" data/test1.csv 100\n";
    std::cout << "\nNote: Only checks rubric criteria:\n";
    std::cout << "  - Area preservation (within 1e-6)\n";
    std::cout << "  - Topology (no self-intersections/ring crossings)\n";
    std::cout << "  - Ring count unchanged\n";
    
    return 0;
}