#ifndef LQFILTERPARSERREACTION_H
#define LQFILTERPARSERREACTION_H

#include "applicationui_export.h"

#include "pqReaction.h"

#include <vtkObject.h>
/**
* @ingroup Reactions
* Reaction to open a pcap
*/
class APPLICATIONUI_EXPORT lqFilterParserReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  lqFilterParserReaction(QAction* action);

protected:
  /// Called when the action is triggered.
  void onTriggered() override;

  static void onProgressEvent(vtkObject* caller, unsigned long, void*);

private:

  class vtkObserver;
  Q_DISABLE_COPY(lqFilterParserReaction)
};

#endif // LQFILTERPARSERREACTION_H
