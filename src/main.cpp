#include "solver.hpp"
#include <iostream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <math.h>       /* sqrt */

int main() {
  Solver solver(6);
  cube_t cube;
  int i;
  char *buf = NULL;
  size_t sz;
  clock_t timer;

  while (getline(&buf, &sz, stdin) != -1) {
	init_cube(&cube);
	if (sz == 0) break;
	rotate_from_str(&cube, buf);		

	dump_cube_grid(&cube);
	timer = clock();
	auto result = solver.solve(cube);
	timer = clock() - timer;
	std::cerr << "Time: " << (float)timer/CLOCKS_PER_SEC << "s\n";

	for (auto rot: result) {
	  std::cerr << rot_to_str(rot) << " ";
	}
	std::cerr << "len: " << result.size() << "\n";

	// for (auto rot: result) {
	//   perform_rotation(&cube, rot);
	// }
	// dump_cube_grid(&cube);
  }

  if (buf)
	free(buf);

}
