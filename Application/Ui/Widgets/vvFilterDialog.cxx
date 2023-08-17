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

#define A0_JSON_FILE "./share/A0-Correction-5.json"
#define A2_JSON_FILE "./share/A2-Correction.json"

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
            item->setGeometry(beginX + i * 70, 20, 70, 20);
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
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 12; j++) {
                auto item = new QCheckBox(widget);
                auto index = i * 12 + j;
                item->setGeometry((width + 10) * i + beginX, (height + 5) * j + beginY, width, height);
                item->setText(QString::number(index));
                item->setProperty("seq", index);
                item->setCheckState(a2Json["channel"].toArray()[index].toInt() == 1 ? Qt::Checked : Qt::Unchecked);
                connect(item, &QCheckBox::clicked, [&, index](int check) {
                    qDebug() << "channel : " << index << " -> " << check;
                    auto array = a2Json["channel"].toArray();
                    array[index] = check;
                    a2Json["channel"] = array;
                });
                this->a2Channel[index] = item;
            }
        }

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
    QCheckBox *a0Module[10];
    QCheckBox *a2Channel[96];
    QCheckBox *face[4];
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
