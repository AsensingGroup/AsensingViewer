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
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QByteArray>
#include <QDebug>

#ifdef __linux__
#ifndef A0_JSON_FILE
#define A0_JSON_FILE "../share/A0-Correction.json"
#endif
#ifndef A2_JSON_FILE
#define A2_JSON_FILE "../share/A2-Correction.json"
#endif
#else
#ifndef A0_JSON_FILE
#define A0_JSON_FILE "./share/A0-Correction.json"
#endif
#ifndef A2_JSON_FILE
#define A2_JSON_FILE "./share/A2-Correction.json"
#endif
#endif
#ifndef A0_CHANNEL_NUMBER
#define A0_CHANNEL_NUMBER 10
#endif
#ifndef A2_CHANNEL_NUMBER
#define A2_CHANNEL_NUMBER 192
#endif
#ifndef BUTTON_FIX_WIDTH
#define BUTTON_FIX_WIDTH 120
#endif

//-----------------------------------------------------------------------------
class vvFilterDialog::pqInternal : public Ui::vvFilterDialog
{
public:
    pqInternal(QDialog *external)
        : Settings(pqApplicationCore::instance()->settings())
    {
        this->External = external;
        this->setupUi(external);

        {
            QFile fileA0(A0_JSON_FILE);
            if(fileA0.open(QIODevice::ReadOnly)) {
                auto message = fileA0.readAll();
                auto data = message.simplified().trimmed();
                QJsonParseError jsonerror;
                auto doc = QJsonDocument::fromJson(data, &jsonerror);
                a0Json = doc.object();
            }
        }

        {
            QFile fileA2(A2_JSON_FILE);
            if(fileA2.open(QIODevice::ReadOnly)) {
                auto message = fileA2.readAll();
                auto data = message.simplified().trimmed();
                QJsonParseError jsonerror;
                auto doc = QJsonDocument::fromJson(data, &jsonerror);
                a2Json = doc.object();
            }
        }

        m_layout = new QVBoxLayout(this->External);
        this->createA0();
        this->createA2();
    }

    ~pqInternal() {
        this->saveSettings();
    }

    void createA0() {
        auto groupA0 = new QGroupBox("A0", this->External);
        groupA0->setFixedHeight(100);
        m_layout->addWidget(groupA0);
        auto widget = groupA0;
        int beginX = 50;
        int beginY = 50;
        int width = 40;
        int height = 20;
        for(int i = 0; i < 10; i++) {
            auto item = new QCheckBox(widget);
            item->setText(QString("module%1").arg(i));
            item->setGeometry(beginX + i * 90, 20, 80, 20);
            item->setProperty("module", i);
            item->setCheckState(a0Json["module"].toArray()[i].toInt() == 1 ? Qt::Checked : Qt::Unchecked);
            connect(item, &QCheckBox::clicked, [&, i](int check) {
                auto array = a0Json["module"].toArray();
                array[i] = check;
                a0Json["module"] = array;
            });
            this->a0Module[i] = item;
        }

        auto filter = new QLineEdit(widget);
        filter->setGeometry(150, 70, 60, 20);
        filter->setText(a0Json["filter_point_id"].toInt() == -1 ? "" : QString::number(a0Json["filter_point_id"].toInt()));
        connect(filter, &QLineEdit::textChanged, [&](const QString &text) {
            a0Json["filter_point_id"] = text.toInt();
        });
        auto label = new QLabel(widget);
        label->setText("Filter Id :");
        label->setGeometry(50, 70, 100, 20);

        selectAllModule = new QPushButton(widget);
        selectAllModule->setText("Select All Module");
        selectAllModule->setGeometry(150 + 100, 70, BUTTON_FIX_WIDTH, 20);
        connect(selectAllModule, &QPushButton::clicked, [=]() {
            auto array = a0Json["module"].toArray();
            for(int i = 0; i < 10; i++) {
                this->a0Module[i]->setChecked(true);
                array[i] = (int)this->a0Module[i]->isChecked();
            }
            a0Json["module"] = array;
        });

        clearAllModule = new QPushButton(widget);
        clearAllModule->setText("Clear All Module");
        clearAllModule->setGeometry(150 + 250, 70, BUTTON_FIX_WIDTH, 20);
        connect(clearAllModule, &QPushButton::clicked, [=]() {
            auto array = a0Json["module"].toArray();
            for(int i = 0; i < 10; i++) {
                this->a0Module[i]->setChecked(false);
                array[i] = (int)this->a0Module[i]->isChecked();
            }
            a0Json["module"] = array;
        });
    }

    void createA2() {
        auto groupA2 = new QGroupBox("A2", this->External);
        m_layout->addWidget(groupA2);
        auto widget = groupA2;
        int beginX = 50;
        int beginY = 50;
        int width = 40;
        int height = 20;
        for(int i = 0; i < 4; i++) {
            auto item = new QCheckBox(widget);
            item->setText(QString("face%1").arg(i));
            item->setGeometry(beginX + i * 70, 20, 70, 20);
            item->setProperty("seq", i);
            item->setCheckState(a2Json["face"].toArray()[i].toInt() == 1 ? Qt::Checked : Qt::Unchecked);
            connect(item, &QCheckBox::clicked, [&, i](int check) {
                auto array = a2Json["face"].toArray();
                array[i] = check;
                a2Json["face"] = array;
            });
            this->face[i] = item;
        }

        selectAllFace = new QPushButton(widget);
        selectAllFace->setText("Select All Face");
        selectAllFace->setGeometry(beginX + 4 * 70 + 80, 20, BUTTON_FIX_WIDTH, 20);
        connect(selectAllFace, &QPushButton::clicked, [=]() {
            auto array = a2Json["face"].toArray();
            for(int i = 0; i < 4; i++) {
                this->face[i]->setChecked(true);
                array[i] = (int)this->face[i]->isChecked();
            }
            a2Json["face"] = array;
        });

        clearAllFace = new QPushButton(widget);
        clearAllFace->setText("Clear All Face");
        clearAllFace->setGeometry(beginX + 4 * 70 + 230, 20, BUTTON_FIX_WIDTH, 20);
        connect(clearAllFace, &QPushButton::clicked, [=]() {
            auto array = a2Json["face"].toArray();
            for(int i = 0; i < 4; i++) {
                this->face[i]->setChecked(false);
                array[i] = (int)this->face[i]->isChecked();
            }
            a2Json["face"] = array;
        });

        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 12; j++) {
                auto item = new QCheckBox(widget);
                auto index = i * 12 + j;
                item->setGeometry((width + 10) * i + beginX, (height + 5) * j + beginY, width, height);
                item->setText(QString::number(index));
                item->setProperty("seq", index);
                auto val = a2Json["channel"].toArray()[index].toInt();
                item->setCheckState(val == 1 ? Qt::Checked : Qt::Unchecked);
                connect(item, &QCheckBox::clicked, [&, index](int check) {
                    auto array = a2Json["channel"].toArray();
                    array[index] = check;
                    a2Json["channel"] = array;
                });
                this->a2Channel[index] = item;
            }
        }

        selectAllChannel = new QPushButton(widget);
        selectAllChannel->setText("Select All Channel");
        selectAllChannel->setGeometry(150 + 80, 12 * 25 + 50, BUTTON_FIX_WIDTH, 20);
        connect(selectAllChannel, &QPushButton::clicked, [=]() {
            auto array = a2Json["channel"].toArray();
            for(int i = 0; i < 16; i++) {
                for(int j = 0; j < 12; j++) {
                    auto index = i * 12 + j;
                    this->a2Channel[index]->setChecked(true);
                    array[index] = 1;
                }
            }
            a2Json["channel"] = array;
        });

        clearAllChannel = new QPushButton(widget);
        clearAllChannel->setText("Clear All Channel");
        clearAllChannel->setGeometry(150 + 230, 12 * 25 + 50, BUTTON_FIX_WIDTH, 20);
        connect(clearAllChannel, &QPushButton::clicked, [=]() {
            auto array = a2Json["channel"].toArray();
            for(int i = 0; i < 16; i++) {
                for(int j = 0; j < 12; j++) {
                    auto index = i * 12 + j;
                    this->a2Channel[index]->setChecked(false);
                    array[index] = 0;
                }
            }
            a2Json["channel"] = array;
        });

        filterId = new QLineEdit(widget);
        filterId->setGeometry(150, 12 * 25 + 50, 60, 20);
        filterId->setText(a2Json["filter_point_id"].toInt() == -1 ? "" : QString::number(a2Json["filter_point_id"].toInt()));
        connect(filterId, &QLineEdit::textChanged, [&](const QString &text) {
            a2Json["filter_point_id"] = text.toInt();
        });
        auto label = new QLabel(widget);
        label->setText("Filter Id :");
        label->setGeometry(50, 12 * 25 + 50, 100, 20);
    }

    void saveSettings() {
        {
            QFile fileA0(A0_JSON_FILE);
            if(fileA0.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(a0Json);
                fileA0.write(doc.toJson(QJsonDocument::Indented));
                fileA0.close();
            }
        }

        {
            QFile fileA2(A2_JSON_FILE);
            if(fileA2.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(a2Json);
                fileA2.write(doc.toJson(QJsonDocument::Indented));
                fileA2.close();
            }
        }
    }
    void restoreSettings() {

    }

    QDialog *External;
    QCheckBox *a0Module[A0_CHANNEL_NUMBER];
    QPushButton *selectAllModule;
    QPushButton *clearAllModule;
    QCheckBox *a2Channel[A2_CHANNEL_NUMBER];
    QCheckBox *face[4];
    QPushButton *selectAllChannel;
    QPushButton *clearAllChannel;
    QPushButton *selectAllFace;
    QPushButton *clearAllFace;
    QLineEdit *filterId;
    pqSettings* const Settings;
    QJsonObject a0Json;
    QJsonObject a2Json;
    QVBoxLayout *m_layout;
};

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
