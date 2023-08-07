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
#include "vvFilterDialog.h"
#include "ui_vvFilterDialog.h"
#include <pqApplicationCore.h>
#include <pqSettings.h>
#include <QPushButton>
#include <QDialog>
#include <QStyle>
#include <sstream>

//-----------------------------------------------------------------------------
class vvFilterDialog::pqInternal : public Ui::vvFilterDialog
{
public:
    pqInternal(QDialog *external)
        : Settings(pqApplicationCore::instance()->settings())
    {
        this->External = external;
        this->setupUi(external);

    }

    void saveSettings();
    void restoreSettings();

    QDialog *External;

    pqSettings* const Settings;
};

//-----------------------------------------------------------------------------
void vvFilterDialog::pqInternal::saveSettings()
{

}

//-----------------------------------------------------------------------------
void vvFilterDialog::pqInternal::restoreSettings()
{

}

//-----------------------------------------------------------------------------
vvFilterDialog::vvFilterDialog(QWidget* p)
    : QDialog(p)
    , Internal(new pqInternal(this))
{

}

//-----------------------------------------------------------------------------
vvFilterDialog::~vvFilterDialog()
{
}
