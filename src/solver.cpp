#include <queue>
#include <iostream>
#include "solver.hpp"

Solver::Solver(uint32_t pruning_tab_depth) {
  pruning_table = kh_init_cube();
  visited_states = kh_init_cube(); 

  generate_pruning_table(pruning_tab_depth);
}

void Solver::generate_pruning_table(uint32_t depth) {
  std::cerr << "Generating pruning table...\n";
  cube_t initial, cube, rotated;
  rot_cnt_t rot_cnt;
  uint16_t rot, cnt;
  int ret;
  khint_t k;
  std::queue<cube_t> q;

  init_cube(&initial);
  k = kh_put_cube(pruning_table, initial, &ret);
  kh_val(pruning_table, k) = build_rot_cnt(NULL_ROT, 0);

  q.push(initial);

  while(!q.empty()) {
	cube = q.front();
	q.pop();

	// std::cerr << "processing cube:\n";
	// dump_cube_grid(&cube);

	k = kh_get_cube(pruning_table, cube);
	rot_cnt = kh_val(pruning_table, k);
	cnt = get_cnt(rot_cnt);
	// std::cerr << cnt << "\n";
	if (cnt >= depth) 
	  continue;
	
	for (rot = ROT_R; rot != NULL_ROT; rot++) {
	  rotated = cube;
	  perform_rotation(&rotated, (rotations)rot); 
	  
	  k = kh_get_cube(pruning_table, rotated);
	  if (k == kh_end(pruning_table)) {
		k = kh_put_cube(pruning_table, rotated, &ret);	
		kh_val(pruning_table, k) = build_rot_cnt(rot, cnt+1);
		q.push(rotated);

		// printf("Cube cnt: %d\n", cnt+1);
		// dump_cube_grid(&rotated);
	  }
	}
  }
  std::cerr << "Generated\n";
}
