#include "src/ui/CEGUIWidget.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h"
#include "ui_CEGUIWidget.h"
#include <qlineedit.h>

CEGUIWidget::CEGUIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CEGUIWidget)
{
    ui->setupUi(this);

    ui->view->setBackgroundRole(QPalette::Dark);

    ui->lblCursorPosition->setText("");
    connect(ui->view, &CEGUIGraphicsView::cursorPositionChanged, [this](int x, int y)
    {
        QString text("Cursor: x %1, y %2");
        ui->lblCursorPosition->setText(text.arg(x).arg(y));
    });

    ui->lblZoom->setText("Zoom: 100%");
    connect(ui->view, &ResizableGraphicsView::zoomChanged, [this](qreal factor)
    {
        QString text("Zoom: %1%");
        ui->lblZoom->setText(text.arg(static_cast<int>(factor * 100.0)));
    });

    connect(ui->resolutionBox->lineEdit(), &QLineEdit::editingFinished, this, &CEGUIWidget::onResolutionTextChanged);
    connect(ui->resolutionBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CEGUIWidget::onResolutionTextChanged);
}

CEGUIWidget::~CEGUIWidget()
{
    delete ui;
}

void CEGUIWidget::setScene(CEGUIGraphicsScene* scene)
{
    ui->view->setScene(scene);

    // Make sure the resolution is set right for the given scene
    onResolutionTextChanged();

    // And mark the view as dirty to force Qt to redraw it
    ui->view->update();
}

CEGUIGraphicsScene* CEGUIWidget::getScene() const
{
    return static_cast<CEGUIGraphicsScene*>(ui->view->scene());
}

CEGUIGraphicsView* CEGUIWidget::getView() const
{
    return ui->view;
}

// The CEGUI view class has several enable/disable features that are very hard to achieve using
// inheritance/composition so they are kept in the CEGUI view class and its base class.
// This method enables/disables various features, calling it with no parameters switches to default.
// wheelZoom - mouse wheel will zoom in and out
// middleButtonScroll - pressing and dragging with the middle button will cause panning/scrolling
// continuousRendering - CEGUI will render continuously (not just when you tell it to)
void CEGUIWidget::setViewFeatures(bool wheelZoom, bool middleButtonScroll, bool continuousRendering)
{
    // Always zoom to the original 100% when changing view features
    ui->view->zoomReset();
    ui->view->setWheelZoomEnabled(wheelZoom);
    ui->view->setMiddleButtonDragScrollEnabled(middleButtonScroll);
    ui->view->setContinuousRendering(continuousRendering);
}

void CEGUIWidget::setResolution(int width, int height)
{
    if (!width && !height)
    {
        ui->resolutionBox->setCurrentText("Project default");
    }
    else
    {
        const QString resolutionStr = QString("%1x%2").arg(width).arg(height);
        ui->resolutionBox->setCurrentText(resolutionStr);
    }
    onResolutionTextChanged();
}

// If you have already activated this container, you can call this to enable CEGUI input propagation
// (The CEGUI instance associated will get mouse and keyboard events if the widget has focus)
void CEGUIWidget::setInputEnabled(bool enable)
{
    if (ui->view)
        ui->view->injectInput(enable);
}

void CEGUIWidget::on_debugInfoButton_clicked()
{
    CEGUIManager::Instance().showDebugInfo();
}

void CEGUIWidget::onResolutionTextChanged()
{
    int width = 0;
    int height = 0;

    auto scene = getScene();

    auto text = ui->resolutionBox->currentText();
    if (text != "Project default") // Special case, leave zeroes for default
    {
        auto sepPos = text.indexOf('x');
        if (sepPos < 0) return;

        // Clamp both to 1 - 4096, should suit 99% of all cases
        bool ok = false;
        width = std::max(1, std::min(4096, text.leftRef(sepPos).toInt(&ok)));
        if (!ok) return;
        height = std::max(1, std::min(4096, text.midRef(sepPos + 1).toInt(&ok)));
        if (!ok) return;

        if (scene &&
            qFuzzyCompare(scene->getContextWidth(), static_cast<float>(width)) &&
            qFuzzyCompare(scene->getContextHeight(), static_cast<float>(height)))
        {
            // Nothing changed
            return;
        }

        const QString resolutionStr = QString("%1x%2").arg(width).arg(height);
        if (ui->resolutionBox->findText(resolutionStr) < 0)
        {
            ui->resolutionBox->insertItem(0, resolutionStr);
            ui->resolutionBox->setCurrentIndex(0);
            return;
        }
    }

    if (scene) scene->setCEGUIDisplaySize(width, height);
}
