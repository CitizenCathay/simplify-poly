#include "polygon.h"
#include "simplify.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <cstdlib>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./simplify <input.csv> <target_vertices>\n";
        return 1;
    }

    const std::string input_path = argv[1];
    const int target = std::atoi(argv[2]);

    VertexPool pool;
    try {
        pool.load_csv(input_path);
    } catch (const std::exception& e) {
        std::cerr << "Error loading CSV: " << e.what() << '\n';
        return 1;
    }

    double area_in = pool.total_signed_area();

    double displacement = simplify(pool, target);

    pool.write_csv();

    double area_out = pool.total_signed_area();

    std::cout << std::scientific << std::setprecision(6);
    std::cout << "Total signed area in input: "  << area_in  << '\n';
    std::cout << "Total signed area in output: " << area_out << '\n';
    std::cout << "Total areal displacement: "    << displacement << '\n';

    return 0;
}