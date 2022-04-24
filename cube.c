#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "khash.h"

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

/******************************************************************************/
/* Macros for generating rotation functions ***********************************/

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

#define ABSTRACT_ROTATION_90(cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3) { \
  (cube).faces[face] = ror((cube).faces[face], len * sizeof(face_t));			\
  face_t temporary_var = (cube).faces[f0];					\
  PASTE_FACE_ROR(cube, f0, f1, M0, M1, C0);					\
  PASTE_FACE_ROR(cube, f1, f2, M1, M2, C1);					\
  PASTE_FACE_ROR(cube, f2, f3, M2, M3, C2);					\
  PASTE_ROR((cube).faces[f3], temporary_var, M3, M0, C3);	\
}

#define ABSTRACT_ROTATION_180(cube, face, f0, M0, f1, M1, f2, M2, f3, M3, C01, C23) {	\
  (cube).faces[face] = ror((cube).faces[face], 4 * sizeof(face_t));	\
  face_t temporary_var = (cube).faces[f0];							\
  PASTE_FACE_ROR(cube, f0, f1, M0, M1, C01);						\
  PASTE_ROR((cube).faces[f1], temporary_var, M1, M0, C01);			\
  temporary_var = (cube).faces[f2];									\
  PASTE_FACE_ROR(cube, f2, f3, M2, M3, C23);						\
  PASTE_ROR((cube).faces[f3], temporary_var, M3, M2, C23);			\
}

#define DEFINE_ROTATION_90(rotation, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3)	\
static void rotation_ ## rotation (cube_t *cube) {	\
  ABSTRACT_ROTATION_90(*cube, face, len, f0, M0, f1, M1, f2, M2, f3, M3, C0, C1, C2, C3);	\
}

#define DEFINE_ROTATION_180(rotation, face, f0, M0, f1, M1, f2, M2, f3, M3, C01, C23)	\
static void rotation_ ## rotation (cube_t *cube) {	\
  ABSTRACT_ROTATION_180(*cube, face, f0, M0, f1, M1, f2, M2, f3, M3, C01, C23);	\
}

/* Clockwise rotations */
DEFINE_ROTATION_90(r, RIGHT, 6, FRONT, 234, DOWN, 234, BACK, 670, UP, 234, 0, 4, 4, 0)
DEFINE_ROTATION_90(l, LEFT, 6, FRONT, 670, UP, 670, BACK, 234, DOWN, 670, 0, 4, 4, 0)
DEFINE_ROTATION_90(u, UP, 6, FRONT, 012, RIGHT, 012, BACK, 012, LEFT, 012, 0, 0, 0, 0)
DEFINE_ROTATION_90(d, DOWN, 6, FRONT, 456, LEFT, 456, BACK, 456, RIGHT, 456, 0, 0, 0, 0)
DEFINE_ROTATION_90(f, FRONT, 6, UP, 456, LEFT, 234, DOWN, 012, RIGHT, 670, 6, 6, 6, 6)
DEFINE_ROTATION_90(b, BACK, 6, UP, 012, RIGHT, 234, DOWN, 456, LEFT, 670, 2, 2, 2, 2)

/* Counter clockwise rotations */
DEFINE_ROTATION_90(rp, RIGHT, 2, FRONT, 234, UP, 234, BACK, 670, DOWN, 234, 0, 4, 4, 0)
DEFINE_ROTATION_90(lp, LEFT, 2, FRONT, 670, DOWN, 670, BACK, 234, UP, 670, 0, 4, 4, 0)
DEFINE_ROTATION_90(up, UP, 2, FRONT, 012, LEFT, 012, BACK, 012, RIGHT, 012, 0, 0, 0, 0)
DEFINE_ROTATION_90(dp, DOWN, 2, FRONT, 456, RIGHT, 456, BACK, 456, LEFT, 456, 0, 0, 0, 0)
DEFINE_ROTATION_90(fp, FRONT, 2, UP, 456, RIGHT, 670, DOWN, 012, LEFT, 234, 2, 2, 2, 2)
DEFINE_ROTATION_90(bp, BACK, 2, UP, 012, LEFT, 670, DOWN, 456, RIGHT, 234, 6, 6, 6, 6)

/* Double clockwise */
DEFINE_ROTATION_180(r2, RIGHT, FRONT, 234, BACK, 670, UP, 234, DOWN, 234, 4, 0)
DEFINE_ROTATION_180(l2, LEFT, FRONT, 670, BACK, 234, UP, 670, DOWN, 670, 4, 0)
DEFINE_ROTATION_180(u2, UP, FRONT, 012, BACK, 012, RIGHT, 012, LEFT, 012, 0, 0)
DEFINE_ROTATION_180(d2, DOWN, FRONT, 456, BACK, 456, RIGHT, 456, LEFT, 456, 0, 0)
DEFINE_ROTATION_180(f2, FRONT, UP, 456, DOWN, 012, RIGHT, 670, LEFT, 234, 4, 4)
DEFINE_ROTATION_180(b2, BACK, UP, 012, DOWN, 456, RIGHT, 234, LEFT, 670, 4, 4)

typedef enum {
  ROT_R = 0,
  ROT_L,
  ROT_U,
  ROT_D,
  ROT_F,
  ROT_B,
  ROT_RP,
  ROT_LP,
  ROT_UP,
  ROT_DP,
  ROT_FP,
  ROT_BP,
  ROT_R2,
  ROT_L2,
  ROT_U2,
  ROT_D2,
  ROT_F2,
  ROT_B2,
  NULL_ROT
} rotations;

typedef void (*rot_f)(cube_t *);

static const rot_f rot_func[] = {
  rotation_r,
  rotation_l,
  rotation_u,
  rotation_d,
  rotation_f,
  rotation_b,
  rotation_rp,
  rotation_lp,
  rotation_up,
  rotation_dp,
  rotation_fp,
  rotation_bp,
  rotation_r2,
  rotation_l2,
  rotation_u2,
  rotation_d2,
  rotation_f2,
  rotation_b2,
};

static void perform_rotations(cube_t *cube, rotations rots[]) {
  while(*rots != NULL_ROT) {
	rot_func[*rots](cube);	
	rots++;
  }
}

/******************************************************************************/
/* Parsing rotations from input ***********************************************/

#define PRIM_CHAR '\''

static int str_to_rot(char *str, rotations *rot) {
  ssize_t len = strlen(str);
  int i;

  if (len == 0 || len > 2) {
	return -1;
  }

  const char rots_char[] = { 'R', 'L', 'U', 'D', 'F', 'B' };
  for (i = 0; i < 6; i++) {
	if (str[0] == rots_char[i]) {
	  *rot = i;
	  if (len == 2) {
		if (str[1] == PRIM_CHAR) {
		  *rot += 6;
		} else if (str[1] == '2') {
		  *rot += 12;
		}
		else {
		  return -1;
		}
	  }
	  return 0;
	}
  }
  return -1;
}

static rotations* parse_scramble(char **str) {
  ssize_t sz = 0;
  ssize_t i = 0;
  char *rot_str = str[0];
  while (rot_str) {
	printf("token: %s\n", rot_str);
	rot_str = str[++sz];
  }

  rotations *rots = calloc(sizeof(rotations), sz+1); 

  for (i = 0; i < sz; i++) {
	if (str_to_rot(str[i], &rots[i])) {
	  free(rots);
	  return NULL;
	}
  }

  rots[sz] = NULL_ROT;

  return rots;
}

static char **tokenize_rot_str(char *str) {
  const char *delims = " \n\t";
  ssize_t capacity = 20;
  char **tokens = calloc(sizeof(char *), capacity);
  int tok_cnt = 0;

  char *tok = strtok(str, delims);
  while (tok != NULL) {
	tok_cnt++;
	if (tok_cnt > capacity) {
	  capacity *= 2;
	  tokens = realloc(tokens, sizeof(char *) * capacity);
	}
	tokens[tok_cnt - 1] = tok;
	tok = strtok(NULL, delims);
  }

  tokens = realloc(tokens, (sizeof(char *) * (1 + tok_cnt)));
  tokens[tok_cnt] = NULL;

  return tokens;
}

static rotations *rot_str_to_rotations(char *str_in) {
  char *str = alloca(strlen(str_in) + 1);
  memcpy(str, str_in, strlen(str_in) + 1);

  char **tokens = tokenize_rot_str(str);
  rotations *rots = parse_scramble(tokens); 
  free(tokens);
  return rots;
}

static int rotate_from_str(cube_t *cube, char *str) {
  rotations *rot = rot_str_to_rotations(str);
  if (!rot) {
	return -1;
  }
  perform_rotations(cube, rot);
  free(rot);
  return 0;
}

static void init_cube(cube_t *cube) {
  for (int face = 0; face < 6; face++) {
	cube->faces[face] = 0;
	for (int tile = 0; tile < 8; tile++) {
	  cube->faces[face] |= (face << (tile * sizeof(face_t)));
	}
  }
}

/******************************************************************************/
/* Printint cube state ********************************************************/

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

/******************************************************************************/
/* Hash map *******************************************************************/

#ifdef FACE32
#define P1 ((uint64_t)516077606561857057)
#define P2 ((uint64_t)217685325020058187) 
#define P3 ((uint64_t)843829397461693519)
#define MOD_M 4200892139
#define first_64(cube) (*(uint64_t*)((cube).faces))
#define second_64(cube) (*(uint64_t*)((cube).faces+2))
#define third_64(cube) (*(uint64_t*)((cube).faces+4))
#define kh_cube_hash_func(cube) (khint32_t)((first_64(cube)*P1+second_64(cube)*P2+third_64(cube)*P3)%(uint64_t)MOD_M)
#define kh_cube_hash_equal(cube1, cube2) (first_64(cube1) == first_64(cube2) && second_64(cube1) == second_64(cube2) && third_64(cube1) == third_64(cube2))

KHASH_INIT(cube, cube_t, int, 0, kh_cube_hash_func, kh_cube_hash_equal)

#endif /* FACE32 */ 


/******************************************************************************/
/* Main ***********************************************************************/

void test_scrambling() {
  cube_t cube;
  int i;
  char *buf = NULL;
  ssize_t sz;

  while (getline(&buf, &sz, stdin) != -1) {
	init_cube(&cube);
	if (sz == 0) break;
	printf("> %s\n", buf);
	rotations *rots = rot_str_to_rotations(buf);
	if (rots) {
	  perform_rotations(&cube, rots);
	  free(rots);
	  dump_cube_grid(&cube);
	} else {
	  printf("Rots == null :o\n");
	}
  }

  if (buf)
	free(buf);
}

int main() {
  int ret;
  khiter_t k;
  cube_t cube;
  khash_t(cube) *h;

  init_cube(&cube);
  h = kh_init(cube);

  kh_put(cube, h, cube, &ret);

  rotate_from_str(&cube, "R D F");  
  
  kh_put(cube, h, cube, &ret);
  for (k = kh_begin(h); k != kh_end(h); ++k) {
	if (kh_exist(h, k)) {
	  dump_cube_grid(&kh_key(h, k));
	}
  }

  kh_destroy(cube, h);
}
