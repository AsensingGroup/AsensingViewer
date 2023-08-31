// Copyright 2014 Velodyne Acoustics, Inc.
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
#ifndef __vvFilterDialog_h
#define __vvFilterDialog_h

#include <QDialog>
#include <QVector3D>

class vvFilterDialog : public QDialog
{
  Q_OBJECT
public:
  vvFilterDialog(QWidget* p = 0);
  virtual ~vvFilterDialog();

private:
  class pqInternal;
  QScopedPointer<pqInternal> Internal;

  Q_DISABLE_COPY(vvFilterDialog)
};

#endif
