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
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonObject>

//-----------------------------------------------------------------------------
class vvFilterDialog::pqInternal : public Ui::vvFilterDialog
{
public:
    pqInternal(QDialog *external)
        : Settings(pqApplicationCore::instance()->settings())
    {
        this->External = external;
        this->setupUi(external);
        this->createA2();
    }

    void createA2() {
        auto layout = new QVBoxLayout(this->External);
        auto groupA2 = new QGroupBox("A2", this->External);
        layout->addWidget(groupA2);

        auto widget = groupA2;
        int beginX = 50;
        int beginY = 50;
        int width = 40;
        int height = 20;

        for(int i = 0; i < 4; i++) {
            auto item = new QCheckBox(widget);
            item->setText(QString("face%1").arg(i));
            item->setGeometry(beginX + i * 70, 20, 70, 20);
            this->face[i] = item;
        }
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 12; j++) {
                auto item = new QCheckBox(widget);
                item->setGeometry((width + 10) * i + beginX, (height + 5) * j + beginY, width, height);
                item->setText(QString::number(i * 12 + j));
                this->channel[i * 12 + j] = item;
            }
        }

        filterId = new QLineEdit(widget);
        filterId->setGeometry(150, 12 * 25 + 50, 60, 20);
        auto label = new QLabel(widget);
        label->setText("Filter Id :");
        label->setGeometry(50, 12 * 25 + 50, 100, 20);
    }

    void saveSettings();
    void restoreSettings();

    QDialog *External;
    QCheckBox *channel[96];
    QCheckBox *face[4];
    QLineEdit *filterId;
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
