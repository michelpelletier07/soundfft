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

MainWindow::~MainWindow() {
    if (m_sndFile != NULL) {
        // Close any previously opened file
        sf_close(m_sndFile);
        m_sndFile = NULL;
        m_sfInfo = {0};
    }
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
    QAction *openAct = new QAction(openIcon, "&Open...", this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip("Open an existing file");
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, "E&xit", this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip("Exit the application");

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAct = helpMenu->addAction("&About", this, &MainWindow::about);
    aboutAct->setStatusTip("Show the application's About box");

    QAction *aboutQtAct = helpMenu->addAction("About &Qt", qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip("Show the Qt library's About box");

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage("Ready");
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
    SF_INFO sfInfo;
    sfInfo.format = 0;
    QByteArray byteArray = fileName.toUtf8();
    const char* c_filename = byteArray.constData();
    SNDFILE *sndFile = sf_open(c_filename, SFM_READ, &sfInfo);
    if (sndFile == NULL) {
        QMessageBox::warning(this, "Application",
            "Cannot read file " + fileName + ":\n" +
            sf_strerror(NULL));
        return;
    }

    // QTextStream in(&file);
    // QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    // // textEdit->setPlainText(in.readAll());
    // QGuiApplication::restoreOverrideCursor();

    if (m_sndFile != NULL) {
        // Close any previously opened file
        sf_close(m_sndFile);
        m_sndFile = NULL;
        m_sfInfo = {0};
    }
    m_sndFile = sndFile;
    m_sfInfo = sfInfo;
    setCurrentFile(fileName);

    QString f_string = "frames: " + QString::number(sfInfo.frames) + "\n" +
                       "samplerate: " + QString::number(sfInfo.samplerate) + "\n" +
                       "channels: " + QString::number(sfInfo.channels) + "\n" +
                       "format: 0x" + QString::number(sfInfo.format, 16) + "\n" +
                       "sections: " + QString::number(sfInfo.sections) + "\n" +
                       "seekable: " + QString::number(sfInfo.seekable) + "\n";
    QMessageBox::information(this, "Application", f_string);
    statusBar()->showMessage("File loaded", 2000);
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
