#ifndef imageplane_h
#define imageplane_h

#include "CommonComponents/drawContext.h"
#include "CommonComponents/ImageTile.h"
#include "CommonComponents/glhelper.h"
#include <vector>

namespace sV2 { class Serializer; }

class ImagePlane: public EditViewObj{
 public:

  // input data 
  int w,h;
  int curslot;
  std::string imagefname[4];
  std::vector<unsigned char> img[4];
  std::vector<float> data_attd;
  float lights[4][3];

  //GLTex2D image_tex;
  ImageTile image_tile;
  //GLTex2D albedo_tex; // not implemented yet!

  GLListHandle circle;

  ImagePlane();
  ~ImagePlane();

  virtual void AskForData(sV2::Serializer *s);
  virtual void TreeScan(TSOCntx *cntx);
  virtual void Draw(DrawCntx *cntx);

  enum {
    image_mode_off,
    image_mode_image,
    image_mode_albedo,

    shape_mode_off,
    shape_mode_notex,
    shape_mode_albedo,
    shape_mode_image,

    edit_mode_off,
    edit_mode_on
  };

  int image_mode;
  int shape_mode;
  int edit_mode;

  GLListHandle shape;
  void BuildShape();

  int  cache_slot;
  void Open(int slot);
  void LoadTxt(int slot);

  GLuint glsel_lightcur;
  GLuint glsel_light[4];
  void DrawLightPoints();

  void Build();
};

#endif
