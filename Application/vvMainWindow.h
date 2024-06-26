// Copyright 2013 Velodyne Acoustics, Inc.
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
#ifndef __vvMainWindow_h
#define __vvMainWindow_h

#include <QMainWindow>

class pqDataRepresentation;

class vvMainWindow : public QMainWindow
{
  Q_OBJECT
  typedef QMainWindow Superclass;

public:
  vvMainWindow();
  virtual ~vvMainWindow() override;

protected:
  void dragEnterEvent(QDragEnterEvent* evt) override;
  void dropEvent(QDropEvent* evt) override;
  void showEvent(QShowEvent* evt) override;
  void closeEvent(QCloseEvent* evt) override;
  bool eventFilter(QObject *obj, QEvent *ev) override;

protected Q_SLOTS:
  void showHelpForProxy(const QString& proxyname, const QString& groupname);
  void handleMessage(const QString&, int);
  //void showWelcomeDialog();
  //void updateFontSize();

  void toggleMVDecoration(); // Toggle Multiview decorations

private:
  Q_DISABLE_COPY(vvMainWindow);

  class pqInternals;
  pqInternals* Internals;

  // Following Methods should be the same accross LidarView-based Apps
  // Exact elements shared accross apps has not been decided yet,
  // coherence has been improved, but code-duplication remains for more freedom.
  void setupPVGUI();      // Common Parts of the ParaViewMainWindow.cxx
  void pqbuildToolbars(); // Reworked pqParaViewMenuBuilders::buildToolbars helper
  void setupLVGUI();      // Add generally common elements to all LidarView-based apps

  void setupGUICustom();  // LidarView Specific UI elements
  void setBranding();     // LidarView Specific Branding

signals:
  void showSpreadSheet();

private:
  Qt::WindowFlags m_windowFlags;
  QDockWidget *m_spreadSheet;
};

#endif
