#include "OpenClDevices.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace apputil {
namespace {

constexpr cl_int platformNotFound = -1001; // CL_PLATFORM_NOT_FOUND_KHR

template<class Handle>
std::string infoString(
  Handle handle, cl_int (*query)(Handle, cl_uint, size_t, void*, size_t*),
  cl_uint property)
{
  size_t size = 0;
  if(query(handle, property, 0, nullptr, &size) != CL_SUCCESS || size == 0)
    return {};
  std::string value(size, '\0');
  if(query(handle, property, size, value.data(), nullptr) != CL_SUCCESS)
    return {};
  while(!value.empty() && value.back() == '\0')
    value.pop_back();
  return value;
}

std::string platformInfo(cl_platform_id platform, cl_platform_info property)
{
  return infoString(platform, clGetPlatformInfo, property);
}

std::string deviceInfo(cl_device_id device, cl_device_info property)
{
  return infoString(device, clGetDeviceInfo, property);
}

bool supportsDoublePrecision(cl_device_id device)
{
  cl_device_fp_config configuration = 0;
  if(clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG,
                     sizeof(configuration), &configuration, nullptr)
       == CL_SUCCESS && configuration != 0)
    return true;

  const std::string extensions = deviceInfo(device, CL_DEVICE_EXTENSIONS);
  return extensions.find("cl_khr_fp64") != std::string::npos
    || extensions.find("cl_amd_fp64") != std::string::npos;
}

} // namespace

std::string OpenClDeviceInfo::description() const
{
  std::ostringstream stream;
  stream << deviceName << " (" << vendor << ", " << platformName << ')';
  if(usesUnifiedMemory)
    stream << " [unified memory]";
  if(supportsDoublePrecision)
    stream << " [FP64]";
  return stream.str();
}

std::vector<OpenClDeviceInfo> enumerateOpenClDevices(
  OpenClDeviceRequirements requirements)
{
  cl_uint platformCount = 0;
  cl_int error = clGetPlatformIDs(0, nullptr, &platformCount);
  if(error == platformNotFound)
    return {};
  if(error != CL_SUCCESS)
    throw std::runtime_error("Could not enumerate OpenCL platforms");

  std::vector<cl_platform_id> platforms(platformCount);
  if(platformCount != 0
     && clGetPlatformIDs(platformCount, platforms.data(), nullptr) != CL_SUCCESS)
    throw std::runtime_error("Could not read the OpenCL platform list");

  std::vector<OpenClDeviceInfo> result;
  for(cl_platform_id platform : platforms) {
    const cl_device_type requestedType = requirements.gpuOnly
      ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_ALL;
    cl_uint deviceCount = 0;
    error = clGetDeviceIDs(platform, requestedType, 0, nullptr, &deviceCount);
    if(error == CL_DEVICE_NOT_FOUND)
      continue;
    if(error != CL_SUCCESS)
      continue;

    std::vector<cl_device_id> devices(deviceCount);
    if(clGetDeviceIDs(
         platform, requestedType, deviceCount, devices.data(), nullptr)
       != CL_SUCCESS)
      continue;

    for(cl_device_id device : devices) {
      cl_bool available = CL_FALSE;
      cl_device_type type = 0;
      cl_bool unifiedMemory = CL_FALSE;
      clGetDeviceInfo(
        device, CL_DEVICE_AVAILABLE, sizeof(available), &available, nullptr);
      clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, nullptr);
      clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY,
                      sizeof(unifiedMemory), &unifiedMemory, nullptr);
      if(available != CL_TRUE)
        continue;

      const bool hasDouble = supportsDoublePrecision(device);
      if(requirements.requireDoublePrecision && !hasDouble)
        continue;

      result.push_back(OpenClDeviceInfo{
        platform,
        device,
        platformInfo(platform, CL_PLATFORM_NAME),
        deviceInfo(device, CL_DEVICE_NAME),
        deviceInfo(device, CL_DEVICE_VENDOR),
        deviceInfo(device, CL_DEVICE_VERSION),
        (type & CL_DEVICE_TYPE_GPU) != 0,
        hasDouble,
        unifiedMemory == CL_TRUE});
    }
  }
  return result;
}

} // namespace apputil
