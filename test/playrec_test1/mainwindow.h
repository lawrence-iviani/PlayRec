#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include "playrec.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    PlayRec m_playrec;
    QFile m_playbckFile;
    bool m_pauseSelect;

    void setUILayout();
    void populateComboBoxes();
    void connectSignals();


private slots:
    void comboBoxPlaybackHasChanged(QString namePBDev);
    void openNewPlaybackFile();
    void playbackStatusHasChanged(int status);
    void pauseToggled();
};

#endif // MAINWINDOW_H
