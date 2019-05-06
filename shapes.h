#define NUM_SHAPES 7

const int l_shape[4][4] =
  { {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  };

const int j_shape[4][4] =
  { {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  };

const int t_shape[4][4] =
  { {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  };

const int square_shape[4][4] =
  { {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  };

const int s_shape[4][4] =
  { {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  };

const int z_shape[4][4] =
  { {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  };

const int bar_shape[4][4] =
  { {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0}
  };

const int shapes[NUM_SHAPES] = {
  l_shape,
  j_shape,
  t_shape,
  square_shape,
  s_shape,
  z_shape,
  bar_shape
};

void transpose(int shape[4][4]) {
  for (int i=0; i<4; i++) {
    for (int j=i+1; j<4; j++) {
      int temp = shape[i][j];
      shape[i][j] = shape[j][i];
      shape[j][i] = temp;
    }
  }
}

void reflect(int shape[4][4]) {
  for (int i=0; i<2; i++) {
    for (int j=0; j<4; j++) {
      int temp = shape[i][j];
      shape[i][j] = shape[3-i][j];
      shape[3-i][j] = temp;
    }
  }
}

void rotate(int shape[4][4], bool clockwise = true) {
  if (clockwise) {
    reflect(shape);
    transpose(shape);
  } else {
    transpose(shape);
    reflect(shape);
  }
}

