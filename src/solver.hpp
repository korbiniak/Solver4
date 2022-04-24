#ifndef _ALGO_HPP_
#define _ALGO_HPP_

#include <vector>

#include "cube.h"

typedef int32_t heur_t;

class Solver {
private:
  kh_cube_t *pruning_table;
  kh_cube_t *visited_states;

  void generate_pruning_table(uint32_t depth);
  std::vector<rotations> retrieve_solution(cube_t cube);
  std::vector<rotations> get_solve_pruning(cube_t cube);
public:
  heur_t heuristic(const cube_t cube, const uint16_t moves_cnt);
  Solver(uint32_t pruning_tab_depth=0);
  ~Solver();
  std::vector<rotations> solve(const cube_t cube);
};

#endif /* _ALGO_HPP_ */
