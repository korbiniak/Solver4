#include <queue>
// #include <priority_queue>
#include <iostream>
#include <algorithm>
#include "solver.hpp"

Solver::Solver(uint32_t pruning_tab_depth) {
  pruning_table = kh_init_cube();
  visited_states = kh_init_cube(); 

  generate_pruning_table(pruning_tab_depth);
}

Solver::~Solver() {
  kh_destroy_cube(pruning_table);
  kh_destroy_cube(visited_states);
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

	k = kh_get_cube(pruning_table, cube);
	rot_cnt = kh_val(pruning_table, k);
	cnt = get_cnt(rot_cnt);
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
	  }
	}
  }

  std::cerr << "Generated\n";
}

std::vector<rotations> Solver::get_solve_pruning(cube_t cube) {
  khiter_t k;
  cube_t initial;
  rotations rot, rev_rot;
  std::vector<rotations> result(0);
  init_cube(&initial);

  k = kh_get_cube(pruning_table, cube);
  if (k == kh_end(pruning_table)) {
	return result;
  }

  while (!kh_cube_hash_equal(cube, initial)) {
	rot = get_rot(kh_val(pruning_table, k));
	rev_rot = reverse_rotation(rot);	
	result.push_back(rev_rot);

	perform_rotation(&cube, rev_rot);
	k = kh_get_cube(pruning_table, cube);
  }

  return result;
}

#define TILE_VAL(i) (face>>((i)<<2)&0xf)
#define CMP_TILES(i, j) (uint32_t)(!!(TILE_VAL(i)==TILE_VAL(j)))

/* Count number of neibourgh tiles of the same color */
__attribute__((optimize("unroll-loops")))
heur_t Solver::heuristic(const cube_t cube, const uint16_t moves_cnt) {
  heur_t value = 0;
  int i, j;
  face_t face;
  for (i = 0; i < 6; i++) {
	face = cube.faces[i];
	for (j = 0; j < 7; j++)
	  value += CMP_TILES(j,j+1);
	value += CMP_TILES(7,0);
	for (j = 0; j < 4; j++)
	  value += (TILE_VAL((j<<1)+1)==i);
  }

  /* This is absolutely random */
  return value * 2 - moves_cnt * 3;
}

typedef std::pair<cube_t, heur_t> qentry_t;
class Comp {
public:
  bool operator()(const qentry_t &e1, const qentry_t &e2) {
	return e1.second < e2.second;
  }
};

std::vector<rotations> Solver::retrieve_solution(cube_t cube) {
  std::vector<rotations> result;
  rot_cnt_t rot_cnt;
  rotations rot, rev_rot;
  khiter_t k;

  k = kh_get_cube(visited_states, cube);
  assert(k != kh_end(visited_states));
  rot_cnt = kh_val(visited_states, k);

  while ((rot = get_rot(rot_cnt)) != NULL_ROT) {
	rev_rot = reverse_rotation(rot); 
	result.push_back(rot);
	perform_rotation(&cube, rev_rot);

	k = kh_get_cube(visited_states, cube);
	rot_cnt = kh_val(visited_states, k);
  }


  std::reverse(result.begin(), result.end());
  return result;
}

std::vector<rotations> Solver::solve(const cube_t cube) {
  cube_t state = cube;
  cube_t new_state;
  khiter_t k;
  rot_cnt_t rot_cnt;
  uint16_t cur_cnt;
  int rot, ret;
  bool found_solution = false;
  std::priority_queue<qentry_t, std::vector<qentry_t>, Comp> pq;
  
  kh_clear_cube(visited_states);
  k = kh_put_cube(visited_states, state, &ret);
  kh_val(visited_states, k) = build_rot_cnt(NULL_ROT, 0);

  pq.push({state, heuristic(cube, 0)});

  while (!found_solution) {
	auto cur = pq.top();
	pq.pop();
	state = cur.first;
	// std::cerr << "Current state:\n";
	// dump_cube_grid(&state);

	k = kh_get_cube(visited_states, state);
	cur_cnt = get_cnt(kh_val(visited_states, k));

	for (rot = ROT_R; rot < NULL_ROT; rot++) {
	  new_state = state;
	  perform_rotation(&new_state, (rotations)rot);

	  k = kh_get_cube(pruning_table, new_state);
	  /* Found the cube in pruning table! We got it :D */
	  if (k != kh_end(pruning_table)) {
		state = new_state;
		found_solution = true;
		k = kh_put_cube(visited_states, new_state, &ret);
		kh_val(visited_states, k) = build_rot_cnt(rot, cur_cnt+1);
		break;
	  }

	  k = kh_get_cube(visited_states, new_state);

	  /* This state was already visited.
	   * Maybe we found a shorter path? */
	  if (k != kh_end(visited_states)) {
		rot_cnt = kh_val(visited_states, k);
		if (get_cnt(rot_cnt) > cur_cnt + 1) {
		  kh_val(visited_states, k) = build_rot_cnt(rot, cur_cnt+1);
		}
	  }
	  /* This is a new unvisited state! */
	  else {
		k = kh_put_cube(visited_states, new_state, &ret);
		kh_val(visited_states, k) = build_rot_cnt(rot, cur_cnt+1);
		pq.push({new_state, heuristic(new_state, cur_cnt+1)});
	  }
	}
  }

  std::cerr << "Found solution!\n" << "\n";
  auto res = retrieve_solution(state);
  auto prun_res = get_solve_pruning(state);
  //std::cerr << prun_res.size() << "\n";
  res.insert(res.end(), prun_res.begin(), prun_res.end()); 
  return res;
}
