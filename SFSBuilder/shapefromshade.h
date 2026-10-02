#ifndef shapefromshade_h
#define shapefromshade_h

#include <array>
#include <vector>

struct DataExchangeBlock {
  DataExchangeBlock(int width, int height);

  int w;
  int h;
  std::array<std::vector<float>, 4> image;
  std::vector<float> albedo;
  std::vector<float> data_norm;
  std::vector<float> data_attd;
};

class ShapeFromShade{
public:
  float s[4][3]; // four sources of illumination
  float s_alb[4][3]; // four sources of illumination for albedo

  void build(DataExchangeBlock& data);
};

#endif
