#include "lqFilterParserReaction.h"

#include <vtkNew.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMParaViewPipelineControllerWithRendering.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>

#include <pqActiveObjects.h>
#include <pqPVApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqSettings.h>
#include <pqView.h>
#include <pqCoreUtilities.h>
#include <pqFileDialog.h>

#include "lqHelper.h"
#include "lqUpdateCalibrationReaction.h"
#include "lqLidarViewManager.h"
#include "vvFilterDialog.h"
#include "lqSensorListWidget.h"

#include <QApplication>
#include <QProgressDialog>
#include <QString>

#include <string>

#include <vtkCommand.h>

#include <vtkProcessModule.h>
#include <vtkPVSession.h>
#include <vtkPVProgressHandler.h>

//----------------------------------------------------------------------------
class lqFilterParserReaction::vtkObserver : public vtkCommand
{
public:
  static vtkObserver* New()
  {
    vtkObserver* obs = new vtkObserver();
    return obs;
  }

  void Execute(vtkObject* , unsigned long eventId, void*) override
  {

      if (eventId == vtkCommand::ProgressEvent)
      {
        QApplication::instance()->processEvents();
      }
  }
};

//-----------------------------------------------------------------------------
lqFilterParserReaction::lqFilterParserReaction(QAction *action) :
  Superclass(action)
{
}

//-----------------------------------------------------------------------------
void lqFilterParserReaction::onTriggered()
{
  vvFilterDialog dialog;
  dialog.exec();
}
