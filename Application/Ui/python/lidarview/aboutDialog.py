# Copyright 2017 Velodyne Acoustics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from __future__ import print_function
from PythonQt import QtCore, QtGui, QtUiTools

def showDialog(mainWindow):

    loader = QtUiTools.QUiLoader()
    uifile = QtCore.QFile(':/vvResources/vvAboutDialog.ui')
    if not uifile.open(uifile.ReadOnly):
        print("Canno't open the ui for the about dialog.")
        return

    dialog = loader.load(uifile, mainWindow)
    uifile.close()
    dialog.setModal(True)

    def w(name):
        for widget in dialog.children():
            if widget.objectName == name:
                return widget

    image = w('splashLabel')
    splash = image.pixmap.scaled(image.pixmap.width()/2.0, image.pixmap.height()/2.0)
    image.setPixmap(splash)
    image.adjustSize()

    appName = mainWindow.windowTitle.split(" ")[0]
    appVersionTag = mainWindow.windowTitle.split(" ")[1]
    appBitTag = mainWindow.windowTitle.split(" ")[2]
    dialog.windowTitle = "About " + appName + " ..."
    copyrightText = '''<h1>{0} {1} {2}</h1><br/>
                       Copyright (c) 2014-2024, Asensing Group, 
                       Provided by <a href="http://www.asensing.com">asensing.com</a><br />
                       <br />
                    '''.format(appName, appVersionTag, appBitTag)
    w('copyrightLabel').setText(copyrightText)
    
    textBoxContent = '''<h4>License</h4>
                        AsensingViewer is provided under the Apache License 2.0.<br /><br />
                        The initial developer of some parts of the framework, which are 
                        copied from, derived from, or inspired by LidarView open source project.
                        <br />
                     '''
    w('detailsLabel').setText(textBoxContent)
    
    
    button = w('closeButton')
    closeIcon = QtGui.QApplication.style().standardIcon(QtGui.QStyle.SP_DialogCloseButton)
    button.setIcon(closeIcon)
    button.connect(button, QtCore.SIGNAL('clicked()'), dialog, QtCore.SLOT('close()'))
    dialog.adjustSize()
    dialog.setFixedSize(dialog.size)

    dialog.exec_()
