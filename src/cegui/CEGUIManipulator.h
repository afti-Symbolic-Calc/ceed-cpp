#ifndef CEGUIMANIPULATOR_H
#define CEGUIMANIPULATOR_H

#include "src/ui/ResizableRectItem.h"
#include <CEGUI/UVector.h>
#include <CEGUI/USize.h>
#include <CEGUI/Sizef.h>
#include "src/QtStdHash.h"
#include <unordered_map>

// This is a rectangle that is synchronised with given CEGUI widget,
// it provides moving and resizing functionality

namespace CEGUI
{
    class Window;
}

class QtnPropertySet;
class QtnProperty;
class QtnPropertyBase;

class CEGUIManipulator : public ResizableRectItem
{
public:

    CEGUIManipulator(QGraphicsItem* parent = nullptr, CEGUI::Window* widget = nullptr);
    virtual ~CEGUIManipulator() override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    virtual QSizeF getMinSize() const override;
    virtual QSizeF getMaxSize() const override;
    CEGUI::Sizef getBaseSize() const;
    QRectF getParentRect() const;

    virtual void notifyHandleSelected(ResizingHandle* handle) override;
    virtual void notifyResizeStarted() override;
    virtual void notifyResizeProgress(QPointF newPos, QRectF newRect) override;
    virtual void notifyResizeFinished(QPointF newPos, QRectF newRect) override;
    virtual void notifyMoveStarted() override;
    virtual void notifyMoveProgress(QPointF newPos) override;
    virtual void notifyMoveFinished(QPointF newPos) override;

    virtual void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false);
    virtual void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true);

    // Returns whether the painting code should strive to prevent manipulator overlap (crossing outlines and possibly other things)
    virtual bool preventManipulatorOverlap() const { return false; }
    virtual bool useAbsoluteCoordsForMove() const { return false; }
    virtual bool useAbsoluteCoordsForResize() const { return false; }
    virtual bool useIntegersForAbsoluteMove() const { return false; }
    virtual bool useIntegersForAbsoluteResize() const { return false; }

    CEGUI::Window* getWidget() const { return _widget; }
    QString getWidgetName() const;
    QString getWidgetType() const;
    QString getWidgetPath() const;
    virtual CEGUIManipulator* createChildManipulator(CEGUI::Window* childWidget, bool recursive = true, bool skipAutoWidgets = false);
    void getChildManipulators(std::vector<CEGUIManipulator*>& outList, bool recursive);
    CEGUIManipulator* getManipulatorByPath(const QString& widgetPath) const;
    CEGUIManipulator* getManipulatorFromChildContainerByPath(const QString& widgetPath) const;
    void forEachChildWidget(std::function<void (CEGUI::Window*)> callback) const;

    void createChildManipulators(bool recursive, bool skipAutoWidgets, bool checkExisting = true);
    void moveToFront();
    bool shouldBeSkipped() const;
    bool hasNonAutoWidgetDescendants() const;

    void updatePropertiesFromWidget(const QStringList& propertyNames);
    void updateAllPropertiesFromWidget();
    QtnPropertySet* getPropertySet() const { return _propertySet; }

    bool isMoveStarted() const { return _moveStarted; }
    void resetMove() { _moveStarted = false; }
    CEGUI::UVector2 getMoveStartPosition() const { return _preMovePos; }

    bool isResizeStarted() const { return _resizeStarted; }
    void resetResize() { _resizeStarted = false; }
    CEGUI::UVector2 getResizeStartPosition() const { return _preResizePos; }
    CEGUI::USize getResizeStartSize() const { return _preResizeSize; }

protected:

    void createPropertySet();

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void onPropertyChanged(const QtnPropertyBase* property, CEGUI::Property* ceguiProperty);
    virtual void impl_paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);

    CEGUI::Window* _widget = nullptr;
    QtnPropertySet* _propertySet = nullptr;
    std::unordered_map<QString, std::pair<CEGUI::Property*, QtnProperty*>> _propertyMap;

    bool _resizeStarted = false;
    CEGUI::UVector2 _preResizePos;
    CEGUI::USize _preResizeSize;
    QPointF _lastResizeNewPos;
    QRectF _lastResizeNewRect;

    bool _moveStarted = false;
    CEGUI::UVector2 _preMovePos;
    QPointF _lastMoveNewPos;
};

#endif // CEGUIMANIPULATOR_H
