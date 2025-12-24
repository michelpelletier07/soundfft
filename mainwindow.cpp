#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget * /* parent */)
{

    createActions();
    createStatusBar();

    readSettings();

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, "About Application",
        "The <b>Application</b> example demonstrates how to "
        "write modern GUI applications using Qt, with a menu bar, "
        "toolbars, and a status bar.");
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QToolBar *fileToolBar = addToolBar("File");

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Application",
            "Cannot read file %1:\n%2.",
            fileName, file.errorString());
        return;
    }

    QTextStream in(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    // textEdit->setPlainText(in.readAll());
    QGuiApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;

    QString shownName = curFile;
    if (curFile.isEmpty()) {
        shownName = "untitled.txt";
    }

    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

MainWindow::~MainWindow()
{

}
