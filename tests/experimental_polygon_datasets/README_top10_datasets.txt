Experimental Polygon Datasets
=================================================

This pack contains 10 custom datasets selected to explore the following topics:
- running time vs input size
- peak memory usage vs input size
- areal displacement vs target vertex count
- topology preservation stress tests
- numerical robustness tests

Recommended usage
-----------------
1. Use files 01-04 for scaling plots and function fitting.
2. Use files 05, 06, and 10 for topology/correctness discussion.
3. Use files 07, 08, and 09 for geometric difficulty and robustness discussion.
4. For areal displacement vs target vertex count, run each dataset at multiple target vertex counts.

File list
---------
- 01_scaling_noisy_circle_10000_vertices.csv
  Target property: High-vertex smooth baseline for runtime and memory scaling.
- 02_scaling_family_circle_1000_vertices.csv
  Target property: Controlled scaling family: same geometry at 1,000 vertices.
- 03_scaling_family_circle_5000_vertices.csv
  Target property: Controlled scaling family: same geometry at 5,000 vertices.
- 04_scaling_family_circle_20000_vertices.csv
  Target property: Controlled scaling family: same geometry at 20,000 vertices.
- 05_topology_many_holes_grid_30.csv
  Target property: Many-hole topology stress test for intersection checks and memory usage.
- 06_topology_serpentine_corridor_4_holes.csv
  Target property: Narrow corridor case that stresses self-intersection rejection.
- 07_geometry_comb_bays_2_holes.csv
  Target property: Thin bays and narrow inlets that tempt invalid shortcuts.
- 08_numerical_near_collinear_outer.csv
  Target property: Near-degenerate outer ring for floating-point and singular-case robustness.
- 09_displacement_starburst_with_3_holes.csv
  Target property: Sharp spikes plus holes for areal-displacement quality and topology interaction.
- 10_topology_wavy_annulus_plus_hole.csv
  Target property: Close outer/inner ring interactions for multi-ring topology preservation.


Why these 10 cases
------------------------
These datasets were selected for coverage rather than quantity:
- a controlled scaling family for fitting asymptotic behavior
- many-hole topology stress
- narrow-gap and close-ring interaction cases
- a near-degenerate numerical case
- a sharp-feature areal-displacement case

Suggested report grouping
-------------------------
Scaling:
  01_scaling_noisy_circle_10000_vertices.csv
  02_scaling_family_circle_1000_vertices.csv
  03_scaling_family_circle_5000_vertices.csv
  04_scaling_family_circle_20000_vertices.csv

Topology stress:
  05_topology_many_holes_grid_30.csv
  06_topology_serpentine_corridor_4_holes.csv
  10_topology_wavy_annulus_plus_hole.csv

Geometry / numerical robustness:
  07_geometry_comb_bays_2_holes.csv
  08_numerical_near_collinear_outer.csv
  09_displacement_starburst_with_3_holes.csv
