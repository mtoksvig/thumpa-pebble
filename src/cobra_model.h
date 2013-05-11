// cobra mk III
// see: http://www.dream-ware.co.uk/berihn/ships

#define MODEL_SCALE 5 // make sure it doesn't go off screen

int32_t verts[][3] = {
  // top (index 0)
  { 0, 0, 4 },
  // front edge (index 1..4)
  { -8, 20, 0 }, { 7, 5, 0 }, { 7, -5, 0 }, { -8, -20, 0 },
  // rear top (index 5..9)
  { -12, 20, 0 }, { -12, 16, 2 }, { -12, 0, 3 }, { -12, -6, 2 },
  { -12, -20, 0 },
  // rear bottom (index 10..11)
  { -12, 6, -2 }, { -12, -6, -2 },
};

int32_t edges[][2] = {
  // edges radiating from top
  { 0, 2 }, { 0, 3 }, { 0, 6 }, { 0, 7 }, { 0, 8 },
  // front edges
  { 5, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 9 },
  // rear polygon
  { 5, 6 }, { 6, 7 }, { 7, 8 }, { 8, 9 }, { 9, 11 }, { 11, 10 }, { 10, 5 },
  // top creases
  { 6, 1 }, { 6, 2 }, { 8, 3 }, { 8, 4 },
  // bottom creases
  { 10, 2 }, { 11, 3 },
};
