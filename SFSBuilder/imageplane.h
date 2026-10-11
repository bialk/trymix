#ifndef imageplane_h
#define imageplane_h

#include "CommonComponents/drawContext.h"
#include "CommonComponents/ImageTile.h"
#include "CommonComponents/ModernOpenGLMesh.h"
#include "shapefromshade.h"
#include <QString>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace sV2 { class Serializer; }
class QObject;

class ImagePlane: public EditViewObj{
 public:

  struct ImageData {
    int width = 0;
    int height = 0;
    std::shared_ptr<const std::vector<unsigned char>> pixels;

    bool empty() const { return !pixels || pixels->empty(); }
    void clear() {
      width = 0;
      height = 0;
      pixels.reset();
    }
  };

  // input data 
  int w,h;
  int curslot;
  std::string imagefname[4];
  std::array<ImageData, 4> images;
  std::vector<float> albedo;
  std::vector<float> data_attd;
  float lights[4][3];

  //GLTex2D image_tex;
  ImageTile image_tile;
  //GLTex2D albedo_tex; // not implemented yet!

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

  ModernOpenGLMesh surfaceMesh;
  ModernOpenGLMesh overlayLineMesh;
  ModernOpenGLMesh overlayPointMesh;
  bool surfaceMeshDirty = true;

  int  cache_slot;
  void Open(int slot);
  void LoadTxt(int slot);
  void SelectImageSlot(int slot);
  void SetShapeMode(int mode);

  int selectedLightIndex = -1;
  void DrawModernSurface(DrawCntx* context);
  void DrawModernLightControls(DrawCntx* context);

  enum class BuildStatus { Succeeded, Failed };
  struct BuildOutput {
    std::vector<float> albedo;
    std::vector<float> surfaceDepth;
  };
  struct BuildResult {
    BuildStatus status = BuildStatus::Failed;
    std::optional<BuildOutput> output;
    QString error;
    qint64 preparationMilliseconds = 0;
    qint64 solverMilliseconds = 0;
    ShapeFromShadeStatistics processingStatistics;
  };
  using BuildCompleted = std::function<void(BuildResult)>;
  void Build(
    QObject* callbackContext, LinSolverKind solverKind, int openClDeviceIndex,
    BuildCompleted completed);
  void ApplyBuildResult(BuildOutput output);
};

#endif
