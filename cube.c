#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  LEFT,
  FRONT,
  RIGHT,
  BACK,
  DOWN
} cube_side;

typedef enum {
  WHITE = 0,
  ORANGE,
  GREEN,
  RED,
  BLUE,
  YELLOW
} colors;

typedef struct {
  face_t faces[6];
} cube_t;

#define ROR(x, y) ((face_t)(x) >> (y) | (face_t)(x) << ((8*sizeof(face_t)) - (y)))
static inline face_t ror(face_t x, face_t y) {
  return ROR(x, y);
}

#define PASTE(to, from, M1, M2) {                                           \
  (from) = ((from) & M ## M1) | ((to) & M ## M2);                           \
}

#define PASTE_ROR(to, from, M1, M2, len) {                                  \
  (from) = ((from) & M ## M1) | ror((to) & M ## M2, len * sizeof(face_t)); \
}

#define PASTE_FACE(cube, to, from, M1, M2) { 		\
  PASTE((cube).faces[to], (cube).faces[from], M1, M2);	\
}

#define PASTE_FACE_ROR(cube, to, from, M1, M2, len) {		\
  PASTE_ROR((cube).faces[to], (cube).faces[from], M1, M2, len);	\
}

#define ABSTRACT_ROTATION(cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3) { \
  ror((cube).faces[face], len * sizeof(face_t));			\
  face_t temporary_var = (cube).faces[f0];					\
  PASTE_FACE_ROR(cube, f0, f1, M0, M1, C0);					\
  PASTE_FACE_ROR(cube, f1, f2, M1, M2, C1);					\
  PASTE_FACE_ROR(cube, f2, f3, M1, M2, C1);					\
  PASTE_FACE_ROR(cube, f3, temporary_var, M1, M2, C1);		\
}

#define DEFINE_ROTATION(rotation, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3)	\
static inline void rotation_ ## rotation (cube_t *cube) {	\
  ABSTRACT_ROTATION(*cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3);	\
}

DEFINE_ROTATION(r, RIGHT, 2, FRONT, 234, DOWN, 234, BACK, 670, UP, 234, 0, 4, 4, 0)

static void init_cube(cube_t *cube) {
  for (int face = 0; face < 6; face++) {
	cube->faces[face] = 0;
	for (int tile = 0; tile < 8; tile++) {
	  cube->faces[face] |= (face << (tile * sizeof(face_t)));
	}
  }
}

static const char letters[] = { 'W', 'O', 'G', 'R', 'B', 'Y' };

static char get_tile(face_t face, int tile) {
  uint8_t color = face >> (tile * sizeof(face_t)) & ((1<<sizeof(face_t)) - 1);
  return letters[color];

  /* switch (color) {
	case WHITE:
	  break;
	case ORANGE:
	case GREEN:
	case RED:
	case BLUE:
	case YELLOW:
  } */
}

static void dump_face(face_t face, char buf[3][3]) {
  buf[0][0] = get_tile(face, 0); 
  buf[0][1] = get_tile(face, 1); 
  buf[0][2] = get_tile(face, 2); 
  buf[1][0] = get_tile(face, 7); 
  buf[1][2] = get_tile(face, 3); 
  buf[2][0] = get_tile(face, 6); 
  buf[2][1] = get_tile(face, 5); 
  buf[2][2] = get_tile(face, 4); 
}

static void fill_face(cube_t *cube, int face, char buf[3][3]) {
  dump_face(cube->faces[face], buf);
  buf[1][1] = letters[face];
}

static void dump_cube_grid(cube_t *cube) {
  int i, j, row, col;
  char buf[3][4][3][3];	/* XD */

  memset(buf, ' ', 3*4*3*3);
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
		  printf("%c", buf[i][j][row][col]);
		} 
	  }
	  printf("\n");
	}
  }
}

int main() {
  cube_t cube;
  init_cube(&cube);
  dump_cube_grid(&cube);
}