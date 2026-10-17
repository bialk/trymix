#ifndef imageplane_h
#define imageplane_h

#include "CommonComponents/drawContext.h"
#include "CommonComponents/ImageTile.h"
#include "CommonComponents/OpenGLMesh.h"
#include "shapefromshade.h"
#include <QString>
#include <QFuture>
#include <array>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace sV2 { class Serializer; }

class ImagePlane: public EditViewObj{
 public:

  struct ImageData {
    int width = 0;
    int height = 0;
    // Copies external pixels into a new immutable allocation. No mutable
    // shared_ptr or moved vector can alias the published buffer.
    static ImageData CopyBGRA(
      int width, int height, std::span<const unsigned char> pixels);

    std::span<const unsigned char> pixels() const {
      return pixels_ ? std::span<const unsigned char>(*pixels_)
                     : std::span<const unsigned char>{};
    }

    bool empty() const { return !pixels_ || pixels_->empty(); }
    void clear() {
      width = 0;
      height = 0;
      pixels_.reset();
    }
  private:
    std::shared_ptr<const std::vector<unsigned char>> pixels_;
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

  OpenGLMesh surfaceMesh;
  OpenGLMesh overlayLineMesh;
  OpenGLMesh overlayPointMesh;
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
  // Snapshot inputs on the UI thread and schedule the calculation on a worker.
  QFuture<BuildResult> Build(LinSolverKind solverKind, int openClDeviceIndex);
  void ApplyBuildResult(BuildOutput output);
};

#endif
