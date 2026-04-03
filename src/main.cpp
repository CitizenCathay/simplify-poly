/* Start Header *****************************************************************/
/*!
\file       main.cpp
\author     Choi Meng Yew, 2401822
\co-author  Chan Qi Ying, 2402302
\co-author  Alyssa Nicole Cerrero Alejandro, 2402435
\date       Apr 03, 2026
\brief
    Implements the program entry point for the polygon simplification
    application.

    This source file parses command-line arguments, loads the input polygon
    from a CSV file, invokes the simplification algorithm, writes the
    simplified polygon to standard output, and reports the input area,
    output area, and total areal displacement.

    It is responsible for basic input validation, error handling during
    file loading, and formatting the final numeric summary in scientific
    notation.
*/
/* End Header *******************************************************************/

#include "polygon.h"
#include "simplify.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <cstdlib>

/*!
\brief
    Runs the polygon simplification program.

\details
    This function expects exactly two command-line arguments: the path to
    the input CSV file and the target maximum number of vertices. It loads
    the polygon data into a vertex pool, computes the signed area of the
    input, runs the simplification algorithm, outputs the simplified
    polygon, and then prints the final area and areal displacement summary.

    If the argument count is incorrect or the input file cannot be loaded,
    the function reports the error to standard error and terminates with a
    non-zero exit code.

\param[in] argc
    The number of command-line arguments.
\param[in] argv
    The array of command-line argument strings.

\return
    Returns 0 on successful execution and 1 if an input or loading error
    occurs.
*/
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
