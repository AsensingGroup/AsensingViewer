//=========================================================================
//
// Copyright 2013,2019 Kitware, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#ifndef GPSPROJECTIONUTILS_H
#define GPSPROJECTIONUTILS_H

#include <string>
#include <vtk_libproj.h>

#include "LidarCoreModule.h"

int LIDARCORE_EXPORT LatLongToZone(double lat, double lon);


class LIDARCORE_EXPORT UTMProjector
{
  public:
  UTMProjector(bool shouldWarnOnWeirdGPSData = false)
    : ShouldWarnOnWeirdGPSData(shouldWarnOnWeirdGPSData) {}

  ~UTMProjector();

  void Project(double lat, double lon, double& easting, double& northing);

  // SignedUTMZone: > 0 means northern hemisphere, < 0 southern
  int SignedUTMZone = 0;

  private:
  bool IsInitialized();

  void Init(double initial_lat, double initial_lon);

  bool ShouldWarnOnWeirdGPSData = true;
  projPJ pj_utm = nullptr;

  std::string UTMString;
};

#endif // GPSPROJECTIONUTILS_H
