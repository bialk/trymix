#pragma once

#include <CL/cl.h>

#include <string>
#include <vector>

namespace apputil {

struct OpenClDeviceRequirements {
  bool gpuOnly = true;
  bool requireDoublePrecision = false;
};

struct OpenClDeviceInfo {
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;
  std::string platformName;
  std::string deviceName;
  std::string vendor;
  std::string version;
  bool isGpu = false;
  bool supportsDoublePrecision = false;
  bool usesUnifiedMemory = false;

  std::string description() const;
};

std::vector<OpenClDeviceInfo> enumerateOpenClDevices(
  OpenClDeviceRequirements requirements = {});

} // namespace apputil
