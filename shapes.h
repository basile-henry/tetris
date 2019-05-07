#define NUM_SHAPES 7
typedef char shape[4][4];

const shape shapes[NUM_SHAPES] = {
  // l_shape
  { {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },

  // j_shape
  { {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  },

  // t_shape
  { {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  },

  // square_shape
  { {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  },

  // s_shape
  { {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  },

  // z_shape
  { {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },

  // bar_shape
  { {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0}
  }
};

void transpose(shape shape) {
  for (int i=0; i<4; i++) {
    for (int j=i+1; j<4; j++) {
      int temp = shape[i][j];
      shape[i][j] = shape[j][i];
      shape[j][i] = temp;
    }
  }
}

void reflect(shape shape) {
  for (int i=0; i<2; i++) {
    for (int j=0; j<4; j++) {
      int temp = shape[i][j];
      shape[i][j] = shape[3-i][j];
      shape[3-i][j] = temp;
    }
  }
}

void rotate(shape shape, bool clockwise = true) {
  if (clockwise) {
    reflect(shape);
    transpose(shape);
  } else {
    transpose(shape);
    reflect(shape);
  }
}
