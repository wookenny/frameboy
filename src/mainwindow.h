/**  frameboy is a simple tool to convert a sequence of images to a video.
*    Copyright (C) 2015  kenny@wook.de
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QGraphicsScene>
#include "qgraphicspixmapitemwatermark.h"
#include "videowriter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showFrame(QListWidgetItem *i);
    void adjustDuration();

    void loadImages();
    void loadWatermark();

private:
    Ui::MainWindow *ui;
    QGraphicsScene      scene_;
    QGraphicsPixmapItemWatermark currentFrame_;
    QString filename_;
    VideoWriterThread thread_;
    VideoWriter vw_;

    void updateSlider_();

private slots:
    void on_actionExit_triggered();
    void on_actionLoad_Images_triggered();
    void on_frameList_itemDoubleClicked(QListWidgetItem *item);
    void on_toolButton_clicked();
    void on_pushButton_clicked();
    void setStatusBar(const QString &string);
    void on_horizontalSlider_valueChanged(int value);
    void on_framerateSpinBox_valueChanged(double value);
    void show_alert(const QString &str);
    void on_delete_button_clicked();
    void on_revert_button_clicked();
    void on_up_button_clicked();
    void on_down_button_clicked();
    void on_load_watermark_clicked();
    void on_doubleSpinBox_2_valueChanged(double arg1);
    void on_doubleSpinBox_valueChanged(double arg1);

    void on_dial_2_valueChanged(int value);
    void on_dial_valueChanged(int value);

    void scaleUpWatermark(int steps);
    void moveWatermark(float x, float y);
    void on_actionLoad_Watermark_triggered();
    void on_buttonAddImages_clicked();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
};


#endif // MAINWINDOW_H
