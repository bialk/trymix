#include "imageplane.h"
#include <FreeImage.h>
#include <QFutureWatcher>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include "apputil/serializerV2.h"
#include "CommonComponents/ModernOpenGLRenderer.h"
#include "shapefromshade.h"

#include <cmath>
#include <utility>
#include <QMatrix4x4>
#include <QVector3D>

ImagePlane::ImagePlane():
  w(0),h(0),
  curslot(0),
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
    img[0].clear();
    img[1].clear();
    img[2].clear();
    img[3].clear();
    curslot = 0;
    cache_slot = -1;
    imagefname[0].clear();
    imagefname[1].clear();
    imagefname[2].clear();
    imagefname[3].clear();
    //image_tex.clear();
    image_tile.clear();
    w=h=0;

    data_attd.clear();
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
  if(cache_slot==slot)  cache_slot = -1;
  
  img[slot].clear();

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
  FIBITMAP* imagen = FreeImage_Load(formato, imagefname[slot].c_str());
  FIBITMAP* temp = imagen;
  if(!imagen){
    qWarning() << "Could not read image:" << imagefname[slot].c_str();
    return;
  }
  imagen = FreeImage_ConvertTo32Bits(imagen);
  if(!imagen) {
    qWarning() << "Could not convert image:" << imagefname[slot].c_str();
    FreeImage_Unload(temp);
    return;
  }
  FreeImage_Unload(temp);
	
  w = FreeImage_GetWidth(imagen);
  h = FreeImage_GetHeight(imagen);

  if(w*h>2048*2048*20) {
    qWarning() << "Image is too large:" << w << 'x' << h;
    return;
  }
  if(0) { // debug code
    printf("load image (%ix%i) for slot %i\n", w, h, slot);
  }  
	

  img[slot].resize(4*w*h);
  GLubyte* img_ptr = &img[slot][0];
  char* pixeles = (char*)FreeImage_GetBits(imagen);
  //FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
  /*	
  for(int j= 0; j<w*h; j++){
    textura[j*4+0]= pixeles[j*4+2];
    textura[j*4+1]= pixeles[j*4+1];
    textura[j*4+2]= pixeles[j*4+0];
    textura[j*4+3]= pixeles[j*4+3];
  }
  */
  memcpy(img_ptr,pixeles,4*w*h);
  FreeImage_Unload(imagen);
}
	
void ImagePlane::LoadTxt(int slot){  
  if(img[slot].empty()) return;
  if( img[slot].size()!=w*h*4 ) {
    qWarning() << "Texture data size does not match image dimensions";
    return;    
  }

  image_tile.LoadBGRA(&img[slot][0], w, h);
  
}

void ImagePlane::DrawModernSurface(DrawCntx* context)
{
  if(data_attd.size() != static_cast<std::size_t>(w * h) || w < 2 || h < 2)
    return;

  if(surfaceMeshDirty) {
    surfaceMesh.destroy();
    std::vector<ModernOpenGLVertex> vertices(static_cast<std::size_t>(w * h));
    std::vector<QVector3D> accumulatedNormals(
      static_cast<std::size_t>(w * h), QVector3D{});
    std::vector<unsigned int> indices;
    indices.reserve(static_cast<std::size_t>((w - 1) * (h - 1) * 6));

    const auto position = [&](int index) {
      return QVector3D(
        static_cast<float>(index % w), static_cast<float>(index / w),
        data_attd[static_cast<std::size_t>(index)]);
    };
    const auto appendTriangle = [&](unsigned int first, unsigned int second,
                                    unsigned int third) {
      indices.insert(indices.end(), {first, second, third});
      const QVector3D normal = QVector3D::crossProduct(
        position(static_cast<int>(second)) - position(static_cast<int>(first)),
        position(static_cast<int>(third)) - position(static_cast<int>(first)));
      accumulatedNormals[first] += normal;
      accumulatedNormals[second] += normal;
      accumulatedNormals[third] += normal;
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

    const bool hasImage = img[0].size() >= static_cast<std::size_t>(w * h * 4);
    for(int index = 0; index < w * h; ++index) {
      const QVector3D point = position(index);
      QVector3D normal = accumulatedNormals[static_cast<std::size_t>(index)];
      if(normal.lengthSquared() > 0.0F)
        normal.normalize();
      else
        normal = {0.0F, 0.0F, 1.0F};

      QVector3D color{0.5F, 0.5F, 0.5F};
      if(hasImage) {
        const auto pixel = static_cast<std::size_t>(index * 4);
        color = {
          img[0][pixel + 2] / 255.0F,
          img[0][pixel + 1] / 255.0F,
          img[0][pixel] / 255.0F};
      }
      vertices[static_cast<std::size_t>(index)] = {
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
  if(image_mode!=image_mode_off){

    if(cache_slot != curslot){
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
  QObject* callbackContext, LinSolverKind solverKind, BuildFinished finished){
  ShapeFromShade sfs(solverKind);
  memcpy(sfs.s,lights,sizeof(sfs.s));
  memcpy(sfs.s_alb,lights,sizeof(sfs.s));

  // preparing data for ShapeFromShade
  DataExchangeBlock data(w, h);

  int i;
  for(i=0;i<w*h;++i){
    data.image[0][i*3+0]=float(img[0][i*4+2])/255.f;
    data.image[0][i*3+1]=float(img[0][i*4+1])/255.f;
    data.image[0][i*3+2]=float(img[0][i*4+0])/255.f;

    data.image[1][i*3+0]=float(img[1][i*4+2])/255.f;
    data.image[1][i*3+1]=float(img[1][i*4+1])/255.f;
    data.image[1][i*3+2]=float(img[1][i*4+0])/255.f;

    data.image[2][i*3+0]=float(img[2][i*4+2])/255.f;
    data.image[2][i*3+1]=float(img[2][i*4+1])/255.f;
    data.image[2][i*3+2]=float(img[2][i*4+0])/255.f;

    data.image[3][i*3+0]=float(img[3][i*4+2])/255.f;
    data.image[3][i*3+1]=float(img[3][i*4+1])/255.f;
    data.image[3][i*3+2]=float(img[3][i*4+0])/255.f;

  }
  auto* watcher = new QFutureWatcher<DataExchangeBlock>(callbackContext);
  QObject::connect(
    watcher, &QFutureWatcher<DataExchangeBlock>::finished, callbackContext,
    [watcher, finished = std::move(finished)]() mutable {
      DataExchangeBlock result = watcher->result();
      watcher->deleteLater();
      finished(std::move(result));
    });

  watcher->setFuture(QtConcurrent::run(
    [sfs = std::move(sfs), data = std::move(data)]() mutable {
      sfs.build(data);
      return data;
    }));
}

void ImagePlane::ApplyBuildResult(DataExchangeBlock data)
{
  data_attd = std::move(data.data_attd);
  surfaceMeshDirty = true;
}
