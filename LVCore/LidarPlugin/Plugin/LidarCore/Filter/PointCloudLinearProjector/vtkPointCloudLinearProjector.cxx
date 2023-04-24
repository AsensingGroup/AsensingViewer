/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPointCloudLinearProjector.cxx
  Author: Pierre Guilbert

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// LOCAL
#include "vtkPointCloudLinearProjector.h"
#include "vtkEigenTools.h"

// STD
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

// VTK
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// BOOST
#include <boost/algorithm/string.hpp>

// Eigen
#include <Eigen/Dense>

// Implementation of the New function
vtkStandardNewMacro(vtkPointCloudLinearProjector)

//------------------------------------------------------------------------------
int vtkPointCloudLinearProjector::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkPointCloudLinearProjector::RequestInformation(vtkInformation *vtkNotUsed(request),
                                                vtkInformationVector **vtkNotUsed(inputVector),
                                                vtkInformationVector *outputVector)
{
  if (this->Resolution[0] <= 0 || this->Resolution[1] <= 0)
  {
    vtkWarningMacro("Resolution must be positive, not " << this->Resolution[0] << "x" << this->Resolution[1] << ".");
    return VTK_ERROR;
  }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               0, this->Resolution[0] - 1,
               0, this->Resolution[1] - 1,
               0, 0);

  outInfo->Set(vtkDataObject::ORIGIN(), this->Origin, 3);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);

  if (this->ExportAsChar)
  {
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 1);
  }
  else
  {
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);
  }
  return VTK_OK;
}

//------------------------------------------------------------------------------
int vtkPointCloudLinearProjector::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
    double h_fov[2] = {-130, 130};
    double v_fov[2] = {-15, 15};
    double v_fov_total = -v_fov[0] + v_fov[1];
    double h_res = 0.1;
    double v_res = 0.2;
    double v_res_rad = v_res * (vtkMath::Pi() / 180);
    double h_res_rad = h_res * (vtkMath::Pi() / 180);
    double y_fudge = 0;
    double x_min = h_fov[0] / h_res / 2;
    double x_max = h_fov[1] / h_res;
    double y_min = v_fov[0] / v_res;
    double y_max = v_fov_total / v_res;
    y_max += y_fudge;
    this->Width = std::floor(x_max / 2 + 1);
    this->Height = std::floor(y_max / 2 + 1);

    // Get the inputs
    vtkPolyData * input = vtkPolyData::GetData(inputVector[0]->GetInformationObject(0));

    // Get the output image and fill with zeros
    vtkImageData* outputImage = vtkImageData::GetData(outputVector->GetInformationObject(0));
    outputImage->SetDimensions(this->Width, this->Height, 1);
    outputImage->SetSpacing(this->Spacing);
    outputImage->SetOrigin(this->Origin);
    outputImage->AllocateScalars(VTK_DOUBLE, 1);
    unsigned char* dataPointer = static_cast<unsigned char*>(outputImage->GetScalarPointer());
    std::fill(dataPointer, dataPointer + this->Height * this->Width, 0);

    // Get the required array
    vtkDataArray* arrayToUse = this->GetInputArrayToProcess(0, inputVector);
    if (!arrayToUse)
    {
        vtkErrorMacro("No input array selected!");
        return 0;
    }
    for (unsigned int indexPoint = 0; indexPoint < input->GetNumberOfPoints(); ++indexPoint)
    {
        double points[3];
        input->GetPoint(indexPoint, points);
        double x_lidar = points[0];
        double y_lidar = points[1];
        double z_lidar = points[2];
        double d_lidar = std::sqrt(x_lidar * x_lidar + y_lidar * y_lidar/* + z_lidar * z_lidar*/);
        double x_img;
        if(y_lidar > 0) {
            x_img = std::atan2(-y_lidar, x_lidar) / h_res_rad;
        }
        else {
            x_img = (- std::atan2(y_lidar, x_lidar)) / h_res_rad;
            std::cout << "+ angle : " << (- std::atan2(y_lidar, x_lidar) / vtkMath::Pi() * 180) << " , " << (x_img - x_min) << std::endl;
        }
        double y_img = std::atan2(z_lidar, d_lidar) / v_res_rad;
        x_img -= x_min;
        y_img -= y_min;
        double pixel_value = /*d_lidar / 255 * 100*/-d_lidar;
        outputImage->SetScalarComponentFromDouble(x_img / 2, y_img / 2, 0, 0, pixel_value);
    }
    return VTK_OK;
}

//------------------------------------------------------------------------------
void vtkPointCloudLinearProjector::SetPlaneNormal(double w0, double w1, double w2)
{
#if 0
  // Here we will construct a new base of R3 using
  // the normal of the plane as the Z-axis. To proceed,
  // we will compute the rotation that map ez toward n
  // and its axis being cross(ez, n)
  Eigen::Vector3d ez(0, 0, 1);
  Eigen::Vector3d n(w0, w1, w2);

  // check that the plane normal is not the null pointer
  if (n.norm() < std::numeric_limits<float>::epsilon())
  {
    vtkGenericWarningMacro("The plane normal cannot be the null vector");
    return;
  }
  n.normalize();

  this->Modified();

  double cosAngle = ez.dot(n);
  Eigen::Vector3d rawAxis = ez.cross(n);
  double rawAxisMagnitude = rawAxis.norm();
  // Check if ez and n are colinear.
  if (rawAxisMagnitude < std::numeric_limits<float>::epsilon())
  {
    // Rotate 180 deg around the x axis if the projection is along negative z.
    if (cosAngle < 0)
    {
      this->ChangeOfBasis << 1,0,0,  0,-1,0,  0,0,-1;
    }
    // Reset it to the identity matrix otherwise.
    else
    {
      this->ChangeOfBasis = Eigen::Matrix3d::Identity();
    }
  }
  else
  {
    Eigen::Vector3d unitAxis = rawAxis / rawAxisMagnitude;
    double sinAngle = rawAxis.dot(unitAxis);
    double angle = std::atan2(sinAngle, cosAngle);
    Eigen::Matrix3d R(Eigen::AngleAxisd(angle, unitAxis));
    this->ChangeOfBasis = R;
  }
  this->Projector = this->DiagonalizedProjector * this->ChangeOfBasis.inverse();
#endif
}
