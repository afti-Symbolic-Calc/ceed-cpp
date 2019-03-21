#include "src/ui/FileSystemBrowser.h"
#include "ui_FileSystemBrowser.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"

FileSystemBrowser::FileSystemBrowser(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FileSystemBrowser)
{
    ui->setupUi(this);

    //???
    // CEED comment (Qt 4.8): causes way too many problems
    //model.setReadOnly(false);
    model.setReadOnly(true);

    auto view = findChild<QListView*>("view");
    view->setModel(&model);


    // Set to project directory if project open, otherwise to user's home
    if (CEGUIProjectManager::Instance().isProjectLoaded())
        setDirectory(CEGUIProjectManager::Instance().getCurrentProject()->getAbsolutePathOf(""));
    else
        setDirectory(QDir::homePath());
}

FileSystemBrowser::~FileSystemBrowser()
{
    delete ui;
}

// Sets the browser to view given directory
void FileSystemBrowser::setDirectory(const QString& dir)
{
    QString absDir = QDir(dir).absolutePath();

    if (!QFileInfo(absDir).isDir()) return;

    directory = absDir;

    model.setRootPath(directory);

    auto view = findChild<QListView*>("view");
    view->setRootIndex(model.index(directory));

    // Add the path to pathBox and select it
    //
    // If the path already exists in the pathBox, remove it and
    // add it to the top.
    // Path comparisons are done case-sensitive because there's
    // no true way to tell if the path is on a case-sensitive
    // file system or not, apart from creating a (temp) file
    // on that file system (and this can't be done once at start-up
    // because the user may have and use multiple file systems).
    auto pathBox = findChild<QComboBox*>("pathBox");
    int existingIndex = pathBox->findText(directory);
    pathBox->blockSignals(true);
    if (existingIndex >= 0)
        pathBox->removeItem(existingIndex);
    pathBox->insertItem(0, directory);
    pathBox->setCurrentIndex(0);
    pathBox->blockSignals(false);
}

QToolButton* FileSystemBrowser::activeFileDirectoryButton() const
{
    return ui->activeFileDirectoryButton;
}

QToolButton* FileSystemBrowser::projectDirectoryButton() const
{
    return ui->projectDirectoryButton;
}

// Slot that gets triggered whenever user double clicks anything in the filesystem view
void FileSystemBrowser::on_view_doubleClicked(const QModelIndex& index)
{
    QString childPath = index.data().toString();
    QString absolutePath = QDir::cleanPath(QDir(directory).filePath(childPath));

    if (QFileInfo(absolutePath).isDir())
        setDirectory(absolutePath);
    else
        emit fileOpenRequested(absolutePath);
}

void FileSystemBrowser::on_parentDirectoryButton_pressed()
{
    setDirectory(QFileInfo(directory).dir().path());
}

void FileSystemBrowser::on_projectDirectoryButton_pressed()
{
    if (CEGUIProjectManager::Instance().isProjectLoaded())
        setDirectory(CEGUIProjectManager::Instance().getCurrentProject()->getAbsolutePathOf(""));
}

void FileSystemBrowser::on_homeDirectoryButton_pressed()
{
    setDirectory(QDir::homePath());
}

void FileSystemBrowser::on_activeFileDirectoryButton_pressed()
{
/*
    if ceed.mainwindow.MainWindow.instance.activeEditor is not None:
        filePath = ceed.mainwindow.MainWindow.instance.activeEditor.filePath
        dirPath = os.path.dirname(filePath)
        self.setDirectory(dirPath)
        # select the active file
        modelIndex = self.model.index(filePath)
        if modelIndex and modelIndex.isValid():
            self.view.setCurrentIndex(modelIndex)
*/
}

// Slot that gets triggered whenever the user selects an path from the list
// or enters a new path and hits enter
void FileSystemBrowser::on_pathBox_currentIndexChanged(int index)
{
    if (index < 0) return;

    // Normally this should be a simple:
    //   self.setDirectory(self.pathBox.currentText())
    // However, when the user edits the text and hits enter, their text
    // is automatically appended to the list of items and this signal
    // is fired. This is fine except that the text may not be a valid
    // directory (typo) and then the pathBox becomes poluted with junk
    // entries.
    // To solve all this, we get the new text, remove the item and then
    // call set directory which will validate and then add the path
    // to the list.
    // The alternative would be to prevent the editted text from being
    // automatically inserted (InsertPolicy(QComboBox::NoInsert)) but
    // then we need custom keyPress handling to detect the enter key
    // press etc (editTextChanged is fired on every keyPress!).
    auto pathBox = findChild<QComboBox*>("pathBox");
    QString newPath = pathBox->currentText();
    pathBox->blockSignals(true);
    pathBox->removeItem(index);
    pathBox->blockSignals(false);
    setDirectory(newPath);
}
