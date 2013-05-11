int32_t munit[3][3] = {
  { 1, 0, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
};

// d = s
void vcopy(int32_t d[3], int32_t s[3]) {
  int i;

  for (i = 0; i < 3; ++i)
    d[i] = s[i];
}

// d = s*factor
void vscale(int32_t d[3], int32_t s[3], int32_t factor) {
  int i;

  for (i = 0; i < 3; ++i)
    d[i] = s[i]*factor;
}

// d = s >> bits
void vshift(int32_t d[3], int32_t s[3], int32_t bits) {
  int i;

  for (i = 0; i < 3; ++i)
    d[i] = s[i] >> bits;
}

// d = s
void mcopy(int32_t d[3][3], int32_t s[3][3]) {
  int i;

  for (i = 0; i < 3; ++i)
    vcopy(d[i], s[i]);
}

// d = s * factor
void mscale(int32_t d[3][3], int32_t s[3][3], int32_t factor) {
  int i;

  for (i = 0; i<3; ++i)
    vscale(d[i], s[i], factor);
}

// d = s >> bits
void mshift(int32_t d[3][3], int32_t s[3][3], int32_t bits) {
  int i;

  for (i = 0; i<3; ++i)
    vshift(d[i], s[i], bits);
}

// d = s * b
// d != s and d != b
void mmul(int32_t d[3][3], int32_t s[3][3], int32_t b[3][3]) {
  int i, j, k;

  for (i = 0; i<3; ++i)
    for (j = 0; j<3; ++j) {
      d[i][j] = 0;
      for (k = 0; k<3; ++k)
        d[i][j] += s[i][k]*b[k][j];
    }
}

// rotate around x axis
// oh, and scale by TRIG_MAX_RATIO
// d != s
void mrotx(int32_t d[3][3], int32_t s[3][3], int32_t angle) {
  int32_t m[3][3] = {
    { TRIG_MAX_RATIO,                  0,                  0 },
    {              0, +cos_lookup(angle), -sin_lookup(angle) },
    {              0, +sin_lookup(angle), +cos_lookup(angle) },
  };

  mmul(d, m, s);
}

// rotate around z axis
// oh, and scale by TRIG_MAX_RATIO
// d != s
void mrotz(int32_t d[3][3], int32_t s[3][3], int32_t angle) {
  int32_t m[3][3] = {
    { +cos_lookup(angle), -sin_lookup(angle),              0 },
    { +sin_lookup(angle), +cos_lookup(angle),              0 },
    {                  0,                  0, TRIG_MAX_RATIO },
  };

  mmul(d, m, s);
}

// d = m.v
void transform(int32_t d[3], int32_t m[3][3], const int32_t v[3]) {
  int i, j;

  for (i = 0; i<3; ++i) {
    d[i] = 0;
    for (j = 0; j<3; ++j)
      d[i] += m[i][j]*v[j];
  }
}

void screen_transform(GPoint *d, int32_t m[3][3], int32_t v[3], int32_t scale) {
  // transform to world
  int32_t w[3]; transform(w, m, v);

  // model is +/-1
  // matrix was scaled by TRIG_MAX_RATIO
  d->x = w[0]*scale/TRIG_MAX_RATIO+SCREEN_CENTER_X;
  d->y = w[1]*scale/TRIG_MAX_RATIO+SCREEN_CENTER_Y;
}
