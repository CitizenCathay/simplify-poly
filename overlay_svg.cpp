#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

struct Point
{
    int vertex_id{};
    double x{};
    double y{};
};

using Rings = std::map<int, std::vector<Point>>;

struct Bounds
{
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double max_y = std::numeric_limits<double>::lowest();

    void include(double x, double y)
    {
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
    }

    bool valid() const
    {
        return min_x <= max_x && min_y <= max_y;
    }
};

static bool starts_with(const std::string& s, const std::string& prefix)
{
    return s.rfind(prefix, 0) == 0;
}

static bool read_polygon_file(const fs::path& path, Rings& rings, Bounds& bounds)
{
    std::ifstream in(path);
    if (!in)
        return false;

    std::string line;
    bool first_line = true;

    while (std::getline(in, line))
    {
        if (line.empty())
            continue;

        if (first_line)
        {
            first_line = false;
            if (starts_with(line, "ring_id"))
                continue;
        }

        if (starts_with(line, "Total "))
            break;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> fields;

        while (std::getline(ss, token, ','))
            fields.push_back(token);

        if (fields.size() != 4)
            continue;

        try
        {
            int ring_id = std::stoi(fields[0]);
            Point p;
            p.vertex_id = std::stoi(fields[1]);
            p.x = std::stod(fields[2]);
            p.y = std::stod(fields[3]);

            rings[ring_id].push_back(p);
            bounds.include(p.x, p.y);
        }
        catch (...)
        {
            continue;
        }
    }

    return !rings.empty() && bounds.valid();
}

template <typename MapX, typename MapY>
static void write_polyline(std::ofstream& out,
                           const std::vector<Point>& points,
                           const std::string& stroke,
                           double stroke_width,
                           MapX map_x,
                           MapY map_y)
{
    if (points.empty())
        return;

    out << "  <polyline fill=\"none\" stroke=\"" << stroke
        << "\" stroke-width=\"" << stroke_width
        << "\" points=\"";

    for (const Point& p : points)
        out << map_x(p.x) << "," << map_y(p.y) << " ";

    out << map_x(points.front().x) << "," << map_y(points.front().y);
    out << "\"/>\n";
}

template <typename MapX, typename MapY>
static void write_points_and_labels(std::ofstream& out,
                                    const std::vector<Point>& points,
                                    const std::string& color,
                                    MapX map_x,
                                    MapY map_y,
                                    bool draw_labels)
{
    for (const Point& p : points)
    {
        const double sx = map_x(p.x);
        const double sy = map_y(p.y);

        out << "  <circle cx=\"" << sx
            << "\" cy=\"" << sy
            << "\" r=\"2.5\" fill=\"" << color << "\"/>\n";

        if (draw_labels)
        {
            out << "  <text x=\"" << (sx + 4.0)
                << "\" y=\"" << (sy - 4.0)
                << "\" font-family=\"Arial\" font-size=\"10\" fill=\"" << color << "\">"
                << p.vertex_id
                << "</text>\n";
        }
    }
}

static bool write_overlay_svg(const fs::path& expected_path,
                              const fs::path& mine_path,
                              const fs::path& out_path,
                              bool draw_labels = true)
{
    Rings expected_rings;
    Rings my_rings;
    Bounds bounds;

    Bounds expected_bounds;
    Bounds my_bounds;

    if (!read_polygon_file(expected_path, expected_rings, expected_bounds))
        return false;

    if (!read_polygon_file(mine_path, my_rings, my_bounds))
        return false;

    bounds.include(expected_bounds.min_x, expected_bounds.min_y);
    bounds.include(expected_bounds.max_x, expected_bounds.max_y);
    bounds.include(my_bounds.min_x, my_bounds.min_y);
    bounds.include(my_bounds.max_x, my_bounds.max_y);

    const double svg_width = 1600.0;
    const double svg_height = 1200.0;
    const double padding = 50.0;

    const double data_width = std::max(1e-9, bounds.max_x - bounds.min_x);
    const double data_height = std::max(1e-9, bounds.max_y - bounds.min_y);

    const double scale_x = (svg_width - 2.0 * padding) / data_width;
    const double scale_y = (svg_height - 2.0 * padding) / data_height;
    const double scale = std::min(scale_x, scale_y);

    auto map_x = [&](double x)
    {
        return padding + (x - bounds.min_x) * scale;
    };

    auto map_y = [&](double y)
    {
        return padding + (bounds.max_y - y) * scale;
    };

    std::ofstream out(out_path);
    if (!out)
        return false;

    out << std::fixed << std::setprecision(3);

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << svg_width << "\" "
        << "height=\"" << svg_height << "\" "
        << "viewBox=\"0 0 " << svg_width << " " << svg_height << "\">\n";

    out << "  <rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    out << "  <text x=\"20\" y=\"28\" font-family=\"Arial\" font-size=\"22\" fill=\"black\">"
        << mine_path.filename().string()
        << "</text>\n";

    out << "  <text x=\"20\" y=\"55\" font-family=\"Arial\" font-size=\"16\" fill=\"red\">"
        << "Expected"
        << "</text>\n";

    out << "  <text x=\"120\" y=\"55\" font-family=\"Arial\" font-size=\"16\" fill=\"black\">"
        << "Mine"
        << "</text>\n";

    for (const auto& [ring_id, points] : expected_rings)
        write_polyline(out, points, "red", 2.0, map_x, map_y);

    for (const auto& [ring_id, points] : my_rings)
        write_polyline(out, points, "black", 1.5, map_x, map_y);

    for (const auto& [ring_id, points] : expected_rings)
        write_points_and_labels(out, points, "red", map_x, map_y, draw_labels);

    for (const auto& [ring_id, points] : my_rings)
        write_points_and_labels(out, points, "black", map_x, map_y, draw_labels);

    out << "</svg>\n";
    return true;
}

int main()
{
    const fs::path expected_dir = "tests/expected-output";
    const fs::path my_dir = "tests/my-output";
    const fs::path out_dir = "tests/svg-overlay";

    const fs::path experimental_expected_dir = "tests/experimental_polygon_datasets";
    const fs::path experimental_my_dir = "tests/my-output/experimental";
    const fs::path experimental_out_dir = "tests/svg-overlay/experimental";

    if (!fs::exists(expected_dir))
    {
        std::cerr << "Missing directory: " << expected_dir << "\n";
        return 1;
    }

    if (!fs::exists(my_dir))
    {
        std::cerr << "Missing directory: " << my_dir << "\n";
        return 1;
    }

    fs::create_directories(out_dir);
    fs::create_directories(experimental_out_dir);

    int generated = 0;
    int skipped = 0;

    // Regular test cases
    for (const auto& entry : fs::directory_iterator(my_dir))
    {
        if (!entry.is_regular_file())
            continue;

        const fs::path my_file = entry.path();
        const fs::path filename = my_file.filename();

        std::string expected_name = filename.stem().string(); // e.g. my_output_original_01
        const std::string prefix = "my_output_";

        if (expected_name.rfind(prefix, 0) == 0)
            expected_name = "output_" + expected_name.substr(prefix.size());

        expected_name += ".txt";

        const fs::path expected_file = expected_dir / expected_name;

        if (!fs::exists(expected_file))
        {
            std::cerr << "Skipping (no expected match): " << filename << "\n";
            ++skipped;
            continue;
        }

        fs::path out_file = out_dir / filename;
        out_file.replace_extension(".svg");

        if (write_overlay_svg(expected_file, my_file, out_file, true))
        {
            std::cout << "Generated: " << out_file << "\n";
            ++generated;
        }
        else
        {
            std::cerr << "Failed: " << filename << "\n";
            ++skipped;
        }
    }

    // Experimental test cases
    if (fs::exists(experimental_expected_dir) && fs::exists(experimental_my_dir))
    {
        for (const auto& entry : fs::directory_iterator(experimental_my_dir))
        {
            if (!entry.is_regular_file())
                continue;

            const fs::path my_file = entry.path();
            const fs::path filename = my_file.filename();

            std::string expected_name = filename.stem().string();
            std::size_t pos = expected_name.rfind("_target_");
            if (pos != std::string::npos)
                expected_name = expected_name.substr(0, pos);

            expected_name += ".csv";

            const fs::path expected_file = experimental_expected_dir / expected_name;

            if (!fs::exists(expected_file))
            {
                std::cerr << "Skipping experimental (no expected match): " << filename << "\n";
                ++skipped;
                continue;
            }

            fs::path out_file = experimental_out_dir / filename;
            out_file.replace_extension(".svg");

            if (write_overlay_svg(expected_file, my_file, out_file, true))
            {
                std::cout << "Generated experimental: " << out_file << "\n";
                ++generated;
            }
            else
            {
                std::cerr << "Failed experimental: " << filename << "\n";
                ++skipped;
            }
        }
    }
    else
    {
        std::cerr << "Skipping experimental overlays (missing directory).\n";
    }

    std::cout << "\nDone. Generated " << generated
              << " overlay SVG(s), skipped " << skipped << ".\n";

    return 0;
}