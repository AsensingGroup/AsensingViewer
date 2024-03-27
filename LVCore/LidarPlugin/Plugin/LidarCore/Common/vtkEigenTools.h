//=========================================================================
//
// Copyright 2018 Kitware, Inc.
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

#ifndef VTK_EIGEN_TOOLS_H
#define VTK_EIGEN_TOOLS_H

// EIGEN
#include <Eigen/Dense>
#include <Eigen/Geometry>

// VTK
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// STD
#include <vector>

#include "LidarCoreModule.h"

/**
   * @brief AvgUnitQuaternions Computes and returns the average unit quaternion
   *        of a unit quaternion list. The average unit quaternion is defined according
   *        to the chordal distance of the unitaries quaternions manifold derived from
   *        the canonical dot product of H
   * @param Q The list of unit quaternion we want to compute the mean
   */
Eigen::Quaterniond AvgUnitQuaternions(const std::vector<Eigen::Quaterniond>& Q);

/**
   * @brief AvgRotation Computes and returns the average rotation of a rotation
   *        list. The average rotation is defined according to the chordal distance
   *        of the SO(3) manifold derived from the Frobenius dot product of M3,3(R)
   * @param rotations The list of rotation we want to compute the mean
   */
Eigen::Matrix3d AvgRotation(const std::vector<Eigen::Matrix3d>& rotations);

/**
   * @brief MatrixToRollPitchYaw Computes the Euler angles from
   *        matrix using the following mapping:
   *        R^3          ->     SO(3)
   *        (rx, ry, rz) -> R = Rz(rz)*Ry(ry)*Rx(rx)
   * with rx denoted as roll and being the angle around X-axis
   * with ry denoted as pitch and being the angle around Y-axis
   * with rz denoted as yaw and being the angle around the Z-axis
   * with (X, Y, Z) being an orthonormal basis of R^3
   * @param rotation input rotation to decompose
   */
Eigen::Vector3d LIDARCORE_EXPORT MatrixToRollPitchYaw(const Eigen::Matrix3d& rotation);

/**
   * @brief RollPitchYawToMatrix Computes the rotation matrix from Euler
   *        angles using the following mapping:
   *        R^3          ->     SO(3)
   *        (rx, ry, rz) -> R = Rz(rz)*Ry(ry)*Rx(rx)
   * with rx denoted as roll and being the angle around X-axis
   * with ry denoted as pitch and being the angle around Y-axis
   * with rz denoted as yaw and being the angle around the Z-axis
   * with (X, Y, Z) being an orthonormal basis of R^3
   * @param roll is the angle around X-axis (in radian)
   * @param pitch is the angle around Y-axis (in radian)
   * @param yaw is the angle around Z-axis (in radian)
   */
Eigen::Matrix3d LIDARCORE_EXPORT RollPitchYawToMatrix(double roll, double pitch, double yaw);
Eigen::Matrix3d LIDARCORE_EXPORT RollPitchYawToMatrix(const Eigen::Vector3d& angles);

/**
  * @brief RollPitchYawInDegreeToMatrix Computes the rotation matrix from Euler
  *        angles using the following mapping:
  *        R^3          ->     SO(3)
  *        (rx, ry, rz) -> R = Rz(rz)*Ry(ry)*Rx(rx)
  * with rx denoted as roll and being the angle around X-axis
  * with ry denoted as pitch and being the angle around Y-axis
  * with rz denoted as yaw and being the angle around the Z-axis
  * with (X, Y, Z) being an orthonormal basis of R^3
  * @param roll is the angle around X-axis (in dedgree)
  * @param pitch is the angle around Y-axis (in dedgree)
  * @param yaw is the angle around Z-axis (in dedgree)
  */
Eigen::Matrix3d LIDARCORE_EXPORT RollPitchYawInDegreeToMatrix(double roll, double pitch, double yaw);

/**
  * @brief SignedAngle Computes the signed angle between two vectors
  * @param v1 first vector
  * @param v2 second vector
  */
double LIDARCORE_EXPORT SignedAngle(const Eigen::Vector3d& v1, const Eigen::Vector3d& v2);

/**
  * @brief AngleFromMatrix Computes the rotation angle (from angle axis representation) of a 3d rotation matrix
  * @param R Rotation Matrix
  */
double LIDARCORE_EXPORT AngleFromMatrix(const Eigen::Matrix3d& R);

/**
  * @brief IsMatrixFinite Check if all the coefficients of a matrix are finite
  * @param M matrix to checkr
  */
bool LIDARCORE_EXPORT IsMatrixFinite(const Eigen::Matrix3d& M);

/**
  * @brief GetSphericalCoordinates compute the spherical coordinates
  *        of the vector (X - Origin) relatively to the basis Basis
  * @param X point we want to compute the spherical coordinate relatively
  *        to the affine space reference frame
  * @param Basis basis part of the affine space reference frame
  *        (basis of the direction vector space)
  * @param Origin origin of the affine space reference frame
  */
Eigen::Vector3d LIDARCORE_EXPORT GetSphericalCoordinates(const Eigen::Vector3d& X,
                                                                 const Eigen::Matrix3d& Basis,
                                                                 const Eigen::Vector3d& Origin);
Eigen::Vector3d LIDARCORE_EXPORT GetSphericalCoordinates(const Eigen::Vector3d& X);

/**
  * @brief ComputeHomography compute the best homography (minimizing
  *        least square error) that maps the x vectors to the y vectors
  * @param x vector of the input vectors
  * @param y vector of the output vectors
  */
Eigen::Matrix3d ComputeHomography(const std::vector<Eigen::Vector2d>& x,
                                  const std::vector<Eigen::Vector2d>& y);

/**
  * @brief EuclideanMLSSmoothing smooth the elements of a euclidean space
  *        using a moving least square algorithm with a polynomial parametric
  *        model function and using the euclidean metric. The kernel function
  *        is a gate(=rectangular) function
  * @param X trajectory to smooth
  * @param Y trajectory smoothed
  * @param polDeg Degree of the polynomial model
  * @param kernelRadius radius of the gate kernel function
  */
void EuclideanMLSSmoothing(const std::vector<Eigen::VectorXd>& X,
                           std::vector<Eigen::VectorXd>& Y,
                           int polDeg, int kernelRadius);

/**
   * @brief MultivariateMedian Computes the multivariate median of a set of
   *        vectors belonging to R^n. The median is obtained by minimizing the
   *        sum of the L2-distance using the Weiszfeld algorithm which is a gradient
   *        descent algorithm with specific step value
   * @param X The list of R^n vectors we want to compute the median
   */
Eigen::VectorXd LIDARCORE_EXPORT MultivariateMedian(const std::vector<Eigen::VectorXd> X, double epsilon = 1e-6, unsigned int maxCount = 10000);

Eigen::Matrix<double, 4, 4> ToEigen(vtkMatrix4x4* M);

Eigen::Matrix<double, 4, 4> ToEigen(vtkTransform* T);

Eigen::Transform<double, 3, Eigen::Affine> ToEigenTransform(vtkMatrix4x4* M);

Eigen::Transform<double, 3, Eigen::Affine> LIDARCORE_EXPORT ToEigenTransform(vtkTransform* T);

Eigen::Transform<double, 3, Eigen::Affine> ToEigenTransform(const Eigen::Matrix3d& R, const Eigen::Vector3d& T);



#endif // VTK_EIGEN_TOOLS_H
