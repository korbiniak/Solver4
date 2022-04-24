#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FACE32
// #define FACE64

#ifdef FACE32
typedef uint32_t face_t;
const uint32_t M012 = 0x00000fff;
const uint32_t M234 = 0x000fff00;
const uint32_t M456 = 0x0fff0000;
const uint32_t M670 = 0xff00000f;
#else
typedef uint64_t face_t;
#endif /* FACE32 */

typedef enum {
  UP = 0,
  LEFT = 1,
  FRONT = 2,
  RIGHT = 3,
  BACK = 4,
  DOWN = 5
} cube_side;

typedef enum {
  WHITE = 0,
  ORANGE = 1,
  GREEN = 2,
  RED = 3,
  BLUE = 4,
  YELLOW = 5,
  EMPTY_COLOR = 6
} colors;

typedef struct {
  face_t faces[6];
} cube_t;

#define ROR(x, y) (face_t)((long)(x) >> (y) | (long)(x) << ((8*sizeof(face_t)) - (y)))
static inline face_t ror(face_t x, face_t y) {
  return ROR(x, y);
}

#define PASTE(to, from, M1, M2) {                                           \
  to = ((to) & ~(M ## M1)) | ((from) & M ## M2);                           \
}

#define PASTE_ROR(to, from, M1, M2, len) {                                  \
  (to) = ((to) & ~(M ## M1)) | ror((from) & M ## M2, len * sizeof(face_t)); \
}

#define PASTE_FACE(cube, to, from, M1, M2) { 		\
  PASTE((cube).faces[to], (cube).faces[from], M1, M2);	\
}

#define PASTE_FACE_ROR(cube, to, from, M1, M2, len) {		\
  PASTE_ROR((cube).faces[to], (cube).faces[from], M1, M2, len);	\
}

#define ABSTRACT_ROTATION(cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3) { \
  (cube).faces[face] = ror((cube).faces[face], len * sizeof(face_t));			\
  face_t temporary_var = (cube).faces[f0];					\
  PASTE_FACE_ROR(cube, f0, f1, M0, M1, C0);					\
  PASTE_FACE_ROR(cube, f1, f2, M1, M2, C1);					\
  PASTE_FACE_ROR(cube, f2, f3, M2, M3, C2);					\
  PASTE_ROR((cube).faces[f3], temporary_var, M3, M0, C3);	\
}

#define DEFINE_ROTATION(rotation, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3)	\
static void rotation_ ## rotation (cube_t *cube) {	\
  ABSTRACT_ROTATION(*cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3);	\
}

DEFINE_ROTATION(r, RIGHT, 6, FRONT, 234, DOWN, 234, BACK, 670, UP, 234, 0, 4, 4, 0)
DEFINE_ROTATION(u, UP, 6, FRONT, 012, RIGHT, 012, BACK, 012, LEFT, 012, 0, 0, 0, 0)

static void init_cube(cube_t *cube) {
  for (int face = 0; face < 6; face++) {
	cube->faces[face] = 0;
	for (int tile = 0; tile < 8; tile++) {
	  cube->faces[face] |= (face << (tile * sizeof(face_t)));
	}
  }
}

static const char letters[] = { 'W', 'O', 'G', 'R', 'B', 'Y' };
static const char *terminal_letters[] = {
  "W",
  "\e[35mO\e[0m",
  "\e[32mG\e[0m",
  "\e[31mR\e[0m",
  "\e[34mB\e[0m",
  "\e[33mY\e[0m",
};

static colors get_tile_color(face_t face, int tile) {
  colors color = face >> (tile * sizeof(face_t)) & ((1<<sizeof(face_t)) - 1);
  assert(color <= 5);
  return color;
}

static void dump_face(face_t face, colors buf[3][3]) {
  buf[0][0] = get_tile_color(face, 0); 
  buf[0][1] = get_tile_color(face, 1); 
  buf[0][2] = get_tile_color(face, 2); 
  buf[1][0] = get_tile_color(face, 7); 
  buf[1][2] = get_tile_color(face, 3); 
  buf[2][0] = get_tile_color(face, 6); 
  buf[2][1] = get_tile_color(face, 5); 
  buf[2][2] = get_tile_color(face, 4); 
  assert(buf[0][0] <= 5);
}

static void fill_face(cube_t *cube, cube_side face, colors buf[3][3]) {
  dump_face(cube->faces[face], buf);
  buf[1][1] = (colors)face;
}

static void dump_cube_grid(cube_t *cube) {
  int i, j, row, col;
  colors buf[3][4][3][3];	/* XD */

  memset(buf, -1, 3*4*3*3*sizeof(colors));
  fill_face(cube, UP, buf[0][1]);
  fill_face(cube, LEFT, buf[1][0]);
  fill_face(cube, FRONT, buf[1][1]);
  fill_face(cube, RIGHT, buf[1][2]);
  fill_face(cube, BACK, buf[1][3]);
  fill_face(cube, DOWN, buf[2][1]);

  for (i = 0; i < 3; i++) {
	for (row = 0; row < 3; row++) {
	  for (j = 0; j < 4; j++) {
		for (col = 0; col < 3; col++) {
		  colors color = buf[i][j][row][col];
		  printf("%s", (color == -1 ? " " : terminal_letters[color])); 
		} 
	  }
	  printf("\n");
	}
  }
}

int main() {
  cube_t cube;
  int i;
  init_cube(&cube);

  for (i = 0; i < 3; i++) {
	rotation_r(&cube);
	rotation_u(&cube);
  }
  dump_cube_grid(&cube);
}
