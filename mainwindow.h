#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <stdio.h>
#include <sndfile.h>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QSessionManager;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadFile(const QString &fileName);

private slots:
    void open();
    void about();

private:
    SNDFILE *m_sndFile = NULL;
    SF_INFO m_sfInfo = {0};

    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QString curFile;

};

#endif // MAINWINDOW_H
