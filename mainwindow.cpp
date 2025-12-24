#include <QtWidgets>
#include <iostream>

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
        QMessageBox::warning(
            this,
            "Application",
            "Cannot read file " + fileName +
                ":\n" + sf_strerror(NULL));
        return;
    }
    if ((sfInfo.channels != 1) &&
        (sfInfo.channels != 2)) {
        QMessageBox::warning(
            this,
            "Application",
            "Unsupported number of channels: " +
                QString::number(sfInfo.channels));
        sf_close(sndFile);
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    // Read all samples into a vector of floats
    std::vector<float> raw_samples(sfInfo.frames * sfInfo.channels);
    sf_read_float(sndFile, raw_samples.data(), sfInfo.frames * sfInfo.channels);
    sf_close(sndFile);

    // Convert from stereo to mono
    std::vector<float> samples = std::vector<float>(sfInfo.frames);
    for (unsigned i = 0; i < sfInfo.frames; i++) {
        float sample = 0.0;
        for (unsigned j = 0; j < sfInfo.channels; j++) {
            sample += raw_samples[(i * sfInfo.channels) + j];
        }
        sample /= sfInfo.channels;
        samples[i] = sample;
    }

    long N = samples.size();
    std::cout << "Read " << N << " frames at " << sfInfo.samplerate << " Hz" << std::endl;

    // FFTW works best with sizes that are products of small factors. The full file size might not be optimal,
    // but we use it for simplicity here. In a real application, you might use an optimal size like 1024 or 2048.
    // Ensure data is aligned for optimal performance.
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    // Convert float samples to fftw_complex input (real part only, imaginary part is 0)
    for (unsigned i = 0; i < N; ++i) {
        in[i][0] = samples[i];  // real part
        in[i][1] = 0.0;         // imaginary part
    }

    // Create a plan for a forward, 1D DFT
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Execute the plan
    fftw_execute(p);
    std::cout << "\nFrequency Magnitudes (first " << N/2 << " bins):" << std::endl;

    // The output array `out` contains complex frequency data. We calculate the magnitude
    // (sqrt(real^2 + imag^2)).
    // Note: for real input, the spectrum is symmetric, so we only need the first N/2 + 1 bins.
    double max_magnitude = 0.0;
    for (int i = 0; i < N / 2; ++i) {
        double real = out[i][0];
        double imag = out[i][1];
        double magnitude = std::sqrt((real * real) + (imag * imag));

        // The frequency for this bin (Hz)
        double frequency_hz = (double)i * sfInfo.samplerate / N;

        // Output results (you would typically use a plotting library to chart these)
        // std::cout << "Frequency: " << frequency_hz << " Hz | Magnitude: " << magnitude << std::endl;

        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
        }
    }

    std::cout << "\nMax magnitude found: " << max_magnitude << std::endl;

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    QGuiApplication::restoreOverrideCursor();

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
