#include "imageplane.h"
#include <FreeImage.h>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include "apputil/serializerV2.h"
#include "CommonComponents/ModernOpenGLRenderer.h"
#include "shapefromshade.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <QMatrix4x4>
#include <QVector3D>

ImagePlane::ImagePlane():
  w(0),h(0),
  curslot(-1),
  image_mode(image_mode_image),
  shape_mode(shape_mode_off),
  edit_mode(edit_mode_off),
  cache_slot(-1)
{
//    imagefname[0] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bej1x4.ppm";
//    imagefname[1] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bej2x4.ppm";
//    imagefname[2] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bej3x4.ppm";
//    imagefname[3] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bej4x4.ppm";
      imagefname[0] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bln1x4.ppm";
      imagefname[1] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bln2x4.ppm";
      imagefname[2] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bln3x4.ppm";
      imagefname[3] = "C:/Users/Alex/D/zero-devel/research/pmetrics/data/bln4x4.ppm";

//    light_vect1=  -0.5   0.4    2
//    light_vect2=   0.5   0.4    2
//    light_vect3=   0.5  -0.4    2
//    light_vect4=  -0.5  -0.4    2

    lights[0][0]=-0.5f; lights[0][1]= 0.4f; lights[0][2]=2.f;
    lights[1][0]= 0.5f; lights[1][1]= 0.4f; lights[1][2]=2.f;
    lights[2][0]= 0.5f; lights[2][1]=-0.4f; lights[2][2]=2.f;
    lights[3][0]=-0.5f; lights[3][1]=-0.4f; lights[3][2]=2.f;

//    lights[0][0]=-1; lights[0][1]=-1; lights[0][2]=4.;
//    lights[1][0]= 1; lights[1][1]=-1; lights[1][2]=4.;
//    lights[2][0]= 1; lights[2][1]= 1; lights[2][2]=4.;
//    lights[3][0]=-1; lights[3][1]= 1; lights[3][2]=4.;
}

ImagePlane::~ImagePlane(){}


void ImagePlane::AskForData(sV2::Serializer *s){

  s->SyncAs("imagefname",imagefname,4);
  s->SyncAs("lights0",lights[0],3);
  s->SyncAs("lights1",lights[1],3);
  s->SyncAs("lights2",lights[2],3);
  s->SyncAs("lights3",lights[3],3);
}

void ImagePlane::TreeScan(TSOCntx *cntx){ 
  if(cntx==&TSOCntx::TSO_Init){
  }
  else if(cntx==&TSOCntx::TSO_ProjectNew){
    for(auto& image : images)
      image.clear();
    curslot = -1;
    cache_slot = -1;
    imagefname[0].clear();
    imagefname[1].clear();
    imagefname[2].clear();
    imagefname[3].clear();
    //image_tex.clear();
    image_tile.clear();
    w=h=0;

    data_attd.clear();
    albedo.clear();
    surfaceMeshDirty = true;
    lights[0][0]=-1; lights[0][1]=-1; lights[0][2]=0;
    lights[1][0]= 1; lights[1][1]=-1; lights[1][2]=0;
    lights[2][0]= 1; lights[2][1]= 1; lights[2][2]=0;
    lights[3][0]=-1; lights[3][1]= 1; lights[3][2]=0;
  }
  else if(cntx==&TSOCntx::TSO_ProjectLoad){
    Open(0);
    Open(1);
    Open(2);
    Open(3);
   }
};


void ImagePlane::Open(int slot){
  if(slot < 0 || slot >= 4)
    return;
  if(cache_slot==slot)  cache_slot = -1;
  
  auto& image = images[slot];
  image.clear();

  struct stat st;
  if(stat(imagefname[slot].c_str(),&st) ) {
    qWarning() << "Image file not found:" << imagefname[slot].c_str();
    return;
  }

  //Format Detecting
  //Automatocally detects the format(from over 20 formats!)
  FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(imagefname[slot].c_str(),0);
  
  if(formato == FIF_UNKNOWN) {
    qWarning() << "Image format is unknown:" << imagefname[slot].c_str();
    return;
  }

  //Image Loading
  using FreeImageBitmap = std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)>;
  FreeImageBitmap source(
    FreeImage_Load(formato, imagefname[slot].c_str()), FreeImage_Unload);
  if(!source){
    qWarning() << "Could not read image:" << imagefname[slot].c_str();
    return;
  }
  FreeImageBitmap bitmap(
    FreeImage_ConvertTo32Bits(source.get()), FreeImage_Unload);
  if(!bitmap) {
    qWarning() << "Could not convert image:" << imagefname[slot].c_str();
    return;
  }

  const auto imageWidth = static_cast<std::size_t>(FreeImage_GetWidth(bitmap.get()));
  const auto imageHeight = static_cast<std::size_t>(FreeImage_GetHeight(bitmap.get()));
  constexpr std::size_t maximumPixelCount = 2048ULL * 2048ULL * 20ULL;
  if(imageWidth == 0 || imageHeight == 0
      || imageWidth > maximumPixelCount / imageHeight) {
    qWarning() << "Invalid image dimensions:" << imageWidth << 'x' << imageHeight;
    return;
  }
  const std::size_t pixelCount = imageWidth * imageHeight;
  if(pixelCount > maximumPixelCount) {
    qWarning() << "Image is too large:" << imageWidth << 'x' << imageHeight;
    return;
  }
  auto loadedPixels = std::make_shared<std::vector<unsigned char>>(4 * pixelCount);
  const BYTE* pixels = FreeImage_GetBits(bitmap.get());
  if(!pixels) {
    qWarning() << "Image contains no pixel data:" << imagefname[slot].c_str();
    image.clear();
    return;
  }
  //FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
  /*	
  for(int j= 0; j<w*h; j++){
    textura[j*4+0]= pixeles[j*4+2];
    textura[j*4+1]= pixeles[j*4+1];
    textura[j*4+2]= pixeles[j*4+0];
    textura[j*4+3]= pixeles[j*4+3];
  }
  */
  memcpy(loadedPixels->data(), pixels, loadedPixels->size());
  image.width = static_cast<int>(imageWidth);
  image.height = static_cast<int>(imageHeight);
  image.pixels = std::move(loadedPixels);
  w = image.width;
  h = image.height;
}
	
void ImagePlane::LoadTxt(int slot){  
  if(slot < 0 || slot >= 4) return;
  const auto& image = images[slot];
  if(image.empty()) return;
  if(image.pixels->size()
      != static_cast<std::size_t>(image.width * image.height * 4)) {
    qWarning() << "Texture data size does not match image dimensions";
    return;    
  }

  image_tile.LoadBGRA(image.pixels->data(), image.width, image.height);
  
}

void ImagePlane::SelectImageSlot(int slot)
{
  if(slot < -1 || slot >= 4 || curslot == slot)
    return;
  curslot = slot;
  if(slot >= 0 && !images[slot].empty()) {
    w = images[slot].width;
    h = images[slot].height;
  }
  surfaceMeshDirty = true;
}

void ImagePlane::SetShapeMode(int mode)
{
  if(shape_mode == mode)
    return;
  shape_mode = mode;
  surfaceMeshDirty = true;
}

void ImagePlane::DrawModernSurface(DrawCntx* context)
{
  if(data_attd.size() != static_cast<std::size_t>(w * h) || w < 2 || h < 2)
    return;

  if(surfaceMeshDirty || !surfaceMesh.isInitialized()) {
    surfaceMesh.destroy();
    std::vector<ModernOpenGLVertex> vertices(static_cast<std::size_t>(w * h));
    std::vector<QVector3D> accumulatedNormals(
      static_cast<std::size_t>(w * h), QVector3D{});
    std::vector<unsigned int> indices;
    indices.reserve(static_cast<std::size_t>((w - 1) * (h - 1) * 6));

    const float* const surfaceData = data_attd.data();
    auto* const vertexData = vertices.data();
    auto* const normalData = accumulatedNormals.data();

    const auto position = [&](int index) {
      return QVector3D(
        static_cast<float>(index % w), static_cast<float>(index / w),
        surfaceData[index]);
    };
    const auto appendTriangle = [&](unsigned int first, unsigned int second,
                                    unsigned int third) {
      indices.insert(indices.end(), {first, second, third});
      const QVector3D normal = QVector3D::crossProduct(
        position(static_cast<int>(second)) - position(static_cast<int>(first)),
        position(static_cast<int>(third)) - position(static_cast<int>(first)));
      normalData[first] += normal;
      normalData[second] += normal;
      normalData[third] += normal;
    };

    for(int y = 0; y < h - 1; ++y) {
      for(int x = 0; x < w - 1; ++x) {
        const auto bottomLeft = static_cast<unsigned int>(y * w + x);
        const auto bottomRight = bottomLeft + 1;
        const auto topLeft = bottomLeft + static_cast<unsigned int>(w);
        const auto topRight = topLeft + 1;
        appendTriangle(bottomLeft, bottomRight, topRight);
        appendTriangle(topRight, topLeft, bottomLeft);
      }
    }

    const bool useImage = shape_mode == shape_mode_image && curslot >= 0
      && !images[curslot].empty()
      && images[curslot].width == w && images[curslot].height == h
      && images[curslot].pixels->size()
           >= static_cast<std::size_t>(w * h * 4);
    const bool useAlbedo = shape_mode == shape_mode_albedo
      && albedo.size() >= static_cast<std::size_t>(w * h * 3);
    const unsigned char* const imagePixels = useImage
      ? images[curslot].pixels->data() : nullptr;
    const float* const albedoPixels = useAlbedo ? albedo.data() : nullptr;
    for(int index = 0; index < w * h; ++index) {
      const QVector3D point = position(index);
      QVector3D normal = normalData[index];
      if(normal.lengthSquared() > 0.0F)
        normal.normalize();
      else
        normal = {0.0F, 0.0F, 1.0F};

      QVector3D color{0.5F, 0.5F, 0.5F};
      if(useImage) {
        const auto pixel = static_cast<std::size_t>(index * 4);
        color = {
          imagePixels[pixel + 2] / 255.0F,
          imagePixels[pixel + 1] / 255.0F,
          imagePixels[pixel] / 255.0F};
      } else if(useAlbedo) {
        const auto pixel = static_cast<std::size_t>(index * 3);
        color = {
          std::clamp(albedoPixels[pixel], 0.0F, 1.0F),
          std::clamp(albedoPixels[pixel + 1], 0.0F, 1.0F),
          std::clamp(albedoPixels[pixel + 2], 0.0F, 1.0F)};
      }
      vertexData[index] = {
        {point.x(), point.y(), point.z()},
        {color.x(), color.y(), color.z()},
        {normal.x(), normal.y(), normal.z()},
        {0.0F, 0.0F}};
    }

    if(!surfaceMesh.initialize(
       vertices.data(), static_cast<int>(vertices.size()),
       indices.data(), static_cast<int>(indices.size())))
      return;
    surfaceMeshDirty = false;
  }

  context->modernRenderer().draw(
    surfaceMesh, context->projectionMatrix(), context->modelViewMatrix());
}

void ImagePlane::DrawModernLightControls(DrawCntx* context)
{
  std::vector<ModernOpenGLVertex> lineVertices;
  std::vector<unsigned int> lineIndices;
  std::vector<ModernOpenGLVertex> pointVertices;
  std::vector<unsigned int> pointIndices;
  const auto appendVertex = [](std::vector<ModernOpenGLVertex>& vertices,
                               std::vector<unsigned int>& indices,
                               const QVector3D& position,
                               const QVector3D& color) {
    indices.push_back(static_cast<unsigned int>(vertices.size()));
    vertices.push_back({
      {position.x(), position.y(), position.z()},
      {color.x(), color.y(), color.z()},
      {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}});
  };

  const QVector3D red{1.0F, 0.0F, 0.0F};
  const QVector3D yellow{1.0F, 1.0F, 0.0F};
  const float centerX = w * 0.5F;
  const float centerY = h * 0.5F;
  appendVertex(lineVertices, lineIndices, {centerX, centerY - 3.0F, 0.0F}, red);
  appendVertex(lineVertices, lineIndices, {centerX, centerY + 3.0F, 0.0F}, red);
  appendVertex(lineVertices, lineIndices, {centerX - 3.0F, centerY, 0.0F}, red);
  appendVertex(lineVertices, lineIndices, {centerX + 3.0F, centerY, 0.0F}, red);

  int selectedLight = -1;
  for(int index = 0; index < 4; ++index) {
    const bool selected = selectedLightIndex == index;
    if(selected)
      selectedLight = index;
    appendVertex(
      pointVertices, pointIndices,
      {(w + lights[index][0] * h) * 0.5F,
       (h + lights[index][1] * h) * 0.5F, 0.0F},
      selected ? yellow : red);
  }

  if(selectedLight >= 0) {
    const float radius = h * 0.5F * std::sqrt(
      lights[selectedLight][0] * lights[selectedLight][0]
      + lights[selectedLight][1] * lights[selectedLight][1]);
    constexpr int circleSegments = 45;
    for(int segment = 0; segment < circleSegments; ++segment) {
      const float angle1 = segment * 2.0F * static_cast<float>(M_PI)
                           / circleSegments;
      const float angle2 = (segment + 1) * 2.0F * static_cast<float>(M_PI)
                           / circleSegments;
      appendVertex(lineVertices, lineIndices,
                   {centerX + radius * std::sin(angle1),
                    centerY + radius * std::cos(angle1), 0.0F}, yellow);
      appendVertex(lineVertices, lineIndices,
                   {centerX + radius * std::sin(angle2),
                    centerY + radius * std::cos(angle2), 0.0F}, yellow);
    }
  }

  if(overlayLineMesh.update(
       lineVertices.data(), static_cast<int>(lineVertices.size()),
       lineIndices.data(), static_cast<int>(lineIndices.size())))
    context->modernRenderer().drawLines(
      overlayLineMesh, context->projectionMatrix(),
      context->modelViewMatrix(), 3.0F);
  if(overlayPointMesh.update(
       pointVertices.data(), static_cast<int>(pointVertices.size()),
       pointIndices.data(), static_cast<int>(pointIndices.size())))
    context->modernRenderer().drawPoints(
      overlayPointMesh, context->projectionMatrix(),
      context->modelViewMatrix(), 6.0F);
}

#if 0
class TaleTxt{
public:
  void Set(unsigned char* image);
  void RenderTriangle(float *vtx, float *txt);  
  void RenderQuadriplet(float *vtx, float *txt);  
};
#endif

void ImagePlane::Draw(DrawCntx *cntx){

  if(w==0 || h==0) return;

  // draw image
  if(image_mode != image_mode_off && curslot >= 0){

    if(cache_slot != curslot || !image_tile.isValidForCurrentContext()){
      cache_slot = curslot;
      LoadTxt(curslot);
    }

    image_tile.Draw(cntx);
  }

  if(shape_mode != shape_mode_off)
    DrawModernSurface(cntx);

  if(image_mode != image_mode_off && edit_mode == edit_mode_on)
    DrawModernLightControls(cntx);

};
  

void ImagePlane::Build(
  QObject* callbackContext, LinSolverKind solverKind,
  BuildCompleted completed){
  ShapeFromShade sfs(solverKind);
  memcpy(sfs.s,lights,sizeof(sfs.s));
  memcpy(sfs.s_alb,lights,sizeof(sfs.s));
  const int buildWidth = w;
  const int buildHeight = h;
  auto sourceImages = images;
  auto* watcher = new QFutureWatcher<BuildResult>(callbackContext);
  QObject::connect(
    watcher, &QFutureWatcher<BuildResult>::finished, callbackContext,
    [watcher, completed = std::move(completed)]() mutable {
      BuildResult result;
      try {
        result = watcher->result();
      }
      catch(const std::exception& error) {
        result.error = QString::fromUtf8(error.what());
      }
      catch(...) {
        result.error = QObject::tr("Unknown error while collecting SFS result");
      }
      watcher->deleteLater();
      completed(std::move(result));
    });

  watcher->setFuture(QtConcurrent::run(
    [sfs = std::move(sfs), sourceImages = std::move(sourceImages),
     buildWidth, buildHeight]() mutable {
      BuildResult result;
      QElapsedTimer timer;
      timer.start();
      try {
        // Allocation and pixel conversion are intentionally part of the worker:
        // for 4K inputs they are large enough to stall the UI thread.
        DataExchangeBlock data(buildWidth, buildHeight);
        std::array<const unsigned char*, 4> sourcePixels;
        std::array<float*, 4> destinationPixels;
        for(int slot = 0; slot < 4; ++slot) {
          sourcePixels[slot] = sourceImages[slot].pixels->data();
          destinationPixels[slot] = data.image[slot].data();
        }

        const int pixelCount = buildWidth * buildHeight;
        for(int i = 0; i < pixelCount; ++i) {
          const auto sourceOffset = static_cast<std::size_t>(i) * 4;
          const auto destinationOffset = static_cast<std::size_t>(i) * 3;
          for(int slot = 0; slot < 4; ++slot) {
            const auto* const source = sourcePixels[slot] + sourceOffset;
            auto* const destination = destinationPixels[slot] + destinationOffset;
            destination[0] = static_cast<float>(source[2]) / 255.0F;
            destination[1] = static_cast<float>(source[1]) / 255.0F;
            destination[2] = static_cast<float>(source[0]) / 255.0F;
          }
        }

        result.preparationMilliseconds = timer.restart();
        sfs.build(data);
        result.solverMilliseconds = timer.elapsed();
        result.data.emplace(std::move(data));
        result.status = BuildStatus::Succeeded;
        qInfo() << "SFS timing: image preparation"
                << result.preparationMilliseconds << "ms, solver"
                << result.solverMilliseconds << "ms, dimensions"
                << buildWidth << 'x' << buildHeight;
      }
      catch(const std::exception& error) {
        result.error = QString::fromUtf8(error.what());
      }
      catch(...) {
        result.error = QObject::tr("Unknown error during SFS calculation");
      }
      return result;
    }));
}

void ImagePlane::ApplyBuildResult(DataExchangeBlock data)
{
  albedo = std::move(data.albedo);
  data_attd = std::move(data.data_attd);
  surfaceMeshDirty = true;
}
