#include "BoxBlur.h"

#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <sstream>
#include <glm/glm.hpp>
#include "apputil/parallelWithBarrier.h"

//##################################################################################################
/*!
 * \param sigma standard deviation
 * \param n number of boxes
 * \return
 */


std::vector<size_t> boxesForGauss(float sigma, size_t n)
{
  // Ideal averaging filter width
  float wIdeal = std::sqrt((12.0f * sigma*sigma / float(n)) + 1.0f);

  size_t wl = size_t(std::floor(wIdeal));
  if(wl % 2 == 0)
    wl--;

  size_t wu = wl + 2;

  auto mIdeal = (12 * sigma*sigma - n * wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
  size_t m = size_t(std::round(mIdeal));

  std::vector<size_t> sizes(n);
  for (size_t i=0; i<n; i++)
    sizes[i] = i < m ? wl : wu;

  return sizes;
}

using float3 = glm::vec3;

inline float3 getf3(float* v, size_t index = 0)
{
  index *= 3;
  return { v[index], v[index+1], v[index+2] };
}

inline void setf3(float* dst, float3 const& v, size_t index = 0)
{
  index *= 3;
  dst[index+0] = v.x;
  dst[index+1] = v.y;
  dst[index+2] = v.z;
}

//##################################################################################################
void boxBlurH_4(float* scl_, float* tcl_, size_t w, size_t h, size_t r)
{
  const int nCnl = 3;
  const size_t nHead = r+1;
  const size_t nTail = r;
  const size_t nBody = w-(nHead+nTail);

  const float iarr = 1.0f / float(r + r + 1);

  std::atomic<std::size_t> c{0};
  parallelWithRunLoop([&](auto /*threadTotal*/, auto /*threadNum*/, auto& /*bwc*/)
  // parallel([&](auto /*threadNum*/)
  {
    for(size_t i = c++;i<h; i=c++)
    {
      float* scl = scl_ + w * i* nCnl;
      float* tcl = tcl_ + w * i* nCnl;

      float* liSCL = scl;
      float* riSCL = scl+r*nCnl;
      float* tiTCL = tcl;

      float3 fv  = getf3(scl);
      float3 lv  = getf3(scl, w - 1);
      float3 val = float(r + 1) * fv;

      for(size_t j=0; j<r; j++)
        val += getf3(scl, j);

      for(float* tiTCLMax=tiTCL+nHead*nCnl; tiTCL<tiTCLMax; riSCL+=nCnl, tiTCL+=nCnl)
      {
        val += getf3(riSCL) - fv;
        setf3(tiTCL, val*iarr);
      }

      for(float* tiTCLMax=tiTCL+nBody*nCnl; tiTCL<tiTCLMax; liSCL+=nCnl, riSCL+=nCnl, tiTCL+=nCnl)
      {
        val += getf3(riSCL) - getf3(liSCL);
        setf3(tiTCL, val*iarr);
      }

      for(float* tiTCLMax=tiTCL+nTail*nCnl; tiTCL<tiTCLMax; liSCL+=nCnl, tiTCL+=nCnl)
      {
        val += lv - getf3(liSCL);
        setf3(tiTCL, val*iarr);
      }
    }
  });
}

//##################################################################################################
void boxBlurT_4(float* scl_, float* tcl_, size_t w, size_t h, size_t r)
{
  const int nCnl = 3;
  const float iarr = 1.0f / float(r + r + 1);

  std::atomic<std::size_t> c{0};
  parallel([&](auto /*threadNum*/)
  {
    for(size_t i = c++; i < w; i = c++){

      float* scl = scl_ + i * nCnl;
      float* tcl = tcl_ + i * nCnl;

      size_t ti = 0;
      size_t li = 0;
      size_t ri = r * w;

      float3 fv = getf3(scl);
      float3 lv = getf3(scl, w * (h - 1));
      float3 val = float(r + 1)*fv;

      for(size_t j=0; j<r; j++)
        val += getf3(scl, j * w);

      for(size_t j=0; j<=r; j++)
      {
        val += getf3(scl, ri) - fv;
        setf3(tcl, val*iarr, ti);
        ri += w; ti += w;
      }

      for(size_t j=r+1; j<h-r; j++)
      {
        val += getf3(scl, ri) - getf3(scl, li);
        setf3(tcl, val*iarr, ti);
        li += w;
        ri += w;
        ti += w;
      }

      for(size_t j=h-r; j<h; j++)
      {
        val += lv-getf3(scl,li);
        setf3(tcl, val*iarr, ti);
        li += w;
        ti += w;
      }
    }
  });
}

//##################################################################################################
void boxBlur_4(float* scl, float* tcl, size_t w, size_t h, size_t r)
{
//  std::memcpy(tcl, scl, sizeof(float3)*w*h);
//  boxBlurH_4(tcl, scl, w, h, r);
//  boxBlurT_4(scl, tcl, w, h, r);
  boxBlurH_4(scl, tcl, w, h, r);
  boxBlurT_4(tcl, scl, w, h, r);
}

////##################################################################################################
void gaussBlur_4_cpu(float* scl, float* tcl, size_t w, size_t h, size_t r)
{
  std::vector<size_t> bxs = boxesForGauss(float(r), 3);
//  boxBlur_4(scl, tcl, w, h, (bxs[0] - 1) / 2);
//  boxBlur_4(tcl, scl, w, h, (bxs[1] - 1) / 2);
//  boxBlur_4(scl, tcl, w, h, (bxs[2] - 1) / 2);
  boxBlur_4(scl, tcl, w, h, (bxs[0] - 1) / 2);
  boxBlur_4(scl, tcl, w, h, (bxs[1] - 1) / 2);
  boxBlur_4(scl, tcl, w, h, (bxs[2] - 1) / 2);
//  std::memcpy(tcl, scl, sizeof(float3)*w*h);
}











#ifdef off_
//##################################################################################################
/*!
 * \param sigma standard deviation
 * \param n number of boxes
 * \return
 */
std::vector<size_t> boxesForGauss(float sigma, size_t n)
{
  // Ideal averaging filter width
  float wIdeal = std::sqrt((12.0f * sigma*sigma / float(n)) + 1.0f);

  size_t wl = size_t(std::floor(wIdeal));
  if(wl % 2 == 0)
    wl--;

  size_t wu = wl + 2;

  auto mIdeal = (12 * sigma*sigma - n * wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
  size_t m = size_t(std::round(mIdeal));

  std::vector<size_t> sizes(n);
  for (size_t i=0; i<n; i++)
    sizes[i] = i < m ? wl : wu;

  return sizes;
}

//##################################################################################################
void boxBlurH_4(float* scl_, float* tcl_, size_t w, size_t h, size_t r)
{
  float3* scl = reinterpret_cast<float3*>(scl_);
  float3* tcl = reinterpret_cast<float3*>(tcl_);
  const size_t nHead = r+1;
  const size_t nTail = r;
  const size_t nBody = w-(nHead+nTail);

  const float iarr = 1.0f / float(r + r + 1);

  std::atomic<std::size_t> c{0};
  parallel([&](auto /*locker*/)
  {
    for(;;)
    {
      size_t const i = c++;

      if(i>=h)
        return;

      {
        size_t ti = i * w;
        size_t li = ti;
        size_t ri = ti + r;

        float3* liSCL = &scl[li];
        float3* riSCL = &scl[ri];
        float3* tiTCL = &tcl[ti];

        float3 fv = scl[ti];
        float3 lv = scl[ti + w - 1];
        float3 val = float(r + 1)*fv;

        for(size_t j=0; j<r; j++)
          val += scl[ti+j];

        for(float3* tiTCLMax=tiTCL+nHead; tiTCL<tiTCLMax; riSCL++, tiTCL++)
        {
          val += *riSCL - fv;
          (*tiTCL) = val*iarr;
        }

        for(float3* tiTCLMax=tiTCL+nBody; tiTCL<tiTCLMax; liSCL++, riSCL++, tiTCL++)
        {
          val += *riSCL - *liSCL;
          (*tiTCL) = val*iarr;
        }

        for(float3* tiTCLMax=tiTCL+nTail; tiTCL<tiTCLMax; liSCL++, tiTCL++)
        {
          val += lv - *liSCL;
          (*tiTCL) = val*iarr;
        }
      }
    }
  });
}

//##################################################################################################
void boxBlurT_4(float* scl_, float* tcl_, size_t w, size_t h, size_t r)
{
  float3* scl = reinterpret_cast<float3*>(scl_);
  float3* tcl = reinterpret_cast<float3*>(tcl_);

  float iarr = 1.0f / float(r + r + 1);

  std::atomic<size_t> c{0};
  parallel([&](auto /*locker*/)
  {
    for(;;)
    {
      size_t const i = c++;

      if(i>=w)
        return;

      size_t ti = i;
      size_t li = ti;
      size_t ri = ti + r * w;

      float3 fv = scl[ti];
      float3 lv = scl[ti + w * (h - 1)];
      float3 val = float(r + 1)*fv;

      for(size_t j=0; j<r; j++)
        val += scl[ti + j * w];

      for(size_t j=0; j<=r; j++)
      {
        val += scl[ri] - fv;
        tcl[ti] = val*iarr;
        ri += w; ti += w;
      }

      for(size_t j=r+1; j<h-r; j++)
      {
        val += scl[ri] - scl[li];
        tcl[ti] = val*iarr;
        li += w;
        ri += w;
        ti += w;
      }

      for(size_t j=h-r; j<h; j++)
      {
        val += lv-scl[li];
        tcl[ti] = val*iarr;
        li += w;
        ti += w;
      }
    }
  });
}

//##################################################################################################
void boxBlur_4(float* scl, float* tcl, size_t w, size_t h, size_t r)
{
  std::memcpy(tcl, scl, sizeof(float3)*w*h);
  boxBlurH_4(tcl, scl, w, h, r);
  boxBlurT_4(scl, tcl, w, h, r);
}

//##################################################################################################
void gaussBlur_4(float* scl, float* tcl, size_t w, size_t h, size_t r)
{
  std::vector<size_t> bxs = boxesForGauss(float(r), 3);
  boxBlur_4(scl, tcl, w, h, (bxs[0] - 1) / 2);
  boxBlur_4(tcl, scl, w, h, (bxs[1] - 1) / 2);
  boxBlur_4(scl, tcl, w, h, (bxs[2] - 1) / 2);
}
#endif
