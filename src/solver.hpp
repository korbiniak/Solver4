#ifndef _ALGO_HPP_
#define _ALGO_HPP_

#include <vector>

#include "cube.h"

class Solver {
private:
  kh_cube_t *pruning_table;
  kh_cube_t *visited_states;

  void generate_pruning_table(uint32_t depth);
public:
  Solver(uint32_t pruning_tab_depth=0);
  std::vector<rotations> solve(cube_t *cube);
};

#endif /* _ALGO_HPP_ */
