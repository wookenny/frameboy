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

#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "graphicsviewscaling.h"
#include "videowriter.h"
#include "common.h"

#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>
#include <QtCore>
#include <QFuture>
#include <QString>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //currentFrame_ = QGraphicsPixmapItemWatermark(parent);
    ui->setupUi(this);
    scene_.addItem(&currentFrame_);
    ui->graphicsView->setScene(&scene_);
    ui->graphicsView->setMainwindow(this);
    setAcceptDrops(true);

    QObject::connect(&vw_,SIGNAL(signalGUI(const QString&)),this,SLOT(setStatusBar(const QString&)));
    QObject::connect(&vw_,SIGNAL(signalError(const QString&)),this,SLOT(show_alert(const QString&)));
    QObject::connect(ui->graphicsView,SIGNAL(signalScaleUp(int)),
                      this,SLOT(scaleUpWatermark(int)));
    QObject::connect(ui->graphicsView,SIGNAL(signalOpacityChange(int)),
                      this,SLOT(changeWatermarkOpacity(int)));
    QObject::connect(ui->graphicsView,SIGNAL(signalMoveTo(float,float)),
                      this,SLOT(moveWatermark(float,float)));
}

MainWindow::~MainWindow()
{
    vw_.exit();
    delete ui;

}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::loadImages( )
{
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                             tr("Select multiple images to open"),
                             QString(), // "/home",
                             tr("Images (*.png *.xpm *.jpg *.jpeg)"));

    loadImages(files);
}

void MainWindow::loadImages(const QStringList& files){
    bool was_empty = ui->frameList->count()==0;
    for(auto it = files.begin(); it!=files.end();++it){
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, removePath(*it) );
        item->setData(Qt::UserRole + 1, (*it));
        item->setToolTip(*it);
        ui->frameList->addItem( item );
    }
    updateSlider_();
    ui->load_watermark->setEnabled(true);
    ui->actionLoad_Watermark->setEnabled(true);

    adjustDuration();
    if(was_empty and ui->frameList->count()>0){
        ui->frameList->setCurrentRow(0);
        showFrame(ui->frameList->item(0));
    }
    setStatusBar("frame images added");
}

void MainWindow::on_actionLoad_Images_triggered()
{
    loadImages();
}

void MainWindow::on_frameList_itemDoubleClicked(QListWidgetItem *item)
{
    ui->frameSlider->setValue( ui->frameList->currentRow() );
    showFrame(item);
}

void MainWindow::on_destination_button_clicked()
{

    filename_ = QFileDialog::getSaveFileName(this,
                            tr("Select the output file"),
                            QString(), // current dir
                            tr("Video (*.avi)")
                            );
    if(filename_=="")//nothing selected!
        return;

    if(!filename_.endsWith(".avi")){
        filename_ += ".avi";
    }

    int width = 20;
    if(filename_.size()<=width)
        ui->destination_button->setText( filename_);
    else
        ui->destination_button->setText("..." + filename_.right(width));
    ui->destination_button->setToolTip(filename_);
}

void MainWindow::on_convertVideo_button_clicked()
{
    vw_.setOutputFilename(filename_);
    vw_.setFramerate(ui->framerateSpinBox->value());
    QStringList list;
    for(int i = 0; i < ui->frameList->count(); ++i)
        list.append( ui->frameList->item(i)->data(Qt::UserRole + 1).toString() );

    vw_.setImages(list);
    vw_.setWatermarkOpacity(ui->opacitySpinbox->value()/100);
    vw_.setWatermarkScale( ui->sizeSpinbox->value()/100);
    vw_.setWatermarkPosX(ui->watermark_x_dial->value());
    vw_.setWatermarkPosY(ui->watermark_y_dial->value());
    setStatusBar("starting video encoding");
    vw_.start();
}

void MainWindow::setStatusBar(const QString &string){
     ui->statusBar->showMessage(string);
}

void MainWindow::showFrame(QListWidgetItem *i)
{
    currentFrame_.setPixmap( QPixmap(i->data((Qt::UserRole + 1)).toString() ));
    ui->graphicsView->setGraphicsSize(currentFrame_.pixmap().width(),currentFrame_.pixmap().height());
    ui->graphicsView->setSceneRect(0, 0, currentFrame_.pixmap().width(), currentFrame_.pixmap().height());
    ui->graphicsView->fitInView(ui->graphicsView->sceneRect(),Qt::KeepAspectRatio);

}

void MainWindow::on_frameSlider_valueChanged(int value)
{
    QListWidgetItem *i = ui->frameList->item(value);
    showFrame(i);
    ui->frameList->setCurrentRow(value);
}

void MainWindow::adjustDuration(){
    double duration = ui->frameList->count();
    duration /= ui->framerateSpinBox->value();
    ui->labelDuration->setText("Duration: "+QString::number(duration,'f',2) + " s");
}

void MainWindow::on_framerateSpinBox_valueChanged(double )
{
    adjustDuration();
}

void MainWindow::show_alert(const QString &str){

    QMessageBox box(this);
    box.setText(str);
    box.setIcon(QMessageBox::Warning);
    box.exec();
}

void MainWindow::on_delete_button_clicked()
{
    qDeleteAll(ui->frameList->selectedItems());
    if(ui->frameList->count()==0){
        ui->load_watermark->setEnabled(false);
        currentFrame_.setPixmap(QPixmap());
    }else
        showFrame(ui->frameList->currentItem());
    updateSlider_();
}

void MainWindow::on_revert_button_clicked()
{
    int num_selected = ui->frameList->selectedItems().count();
    int top = 0, bottom = ui->frameList->count()-1;
    while(num_selected > 1)
    {
        //get top selected
        while(not ui->frameList->item(top)->isSelected())
            ++top;
        //get bottom selected
        while(not ui->frameList->item(bottom)->isSelected())
            --bottom;
        //swap pairs
        QListWidgetItem *item = ui->frameList->takeItem(top);
        ui->frameList->insertItem(bottom-1, item);
        ui->frameList->item(bottom-1)->setSelected(true);
        item = ui->frameList->takeItem(bottom);
        ui->frameList->insertItem(top, item);
        ui->frameList->item(top)->setSelected(true);

        //increment both
        ++top;
        --bottom;
        num_selected -= 2;
    }
    if(ui->frameList->currentItem() != nullptr)
        showFrame(ui->frameList->currentItem());
    updateSlider_();
    ui->frameList->setFocus();
}

void MainWindow::on_up_button_clicked()
{

    int do_not_move = true;
    for(int row = 0; row < ui->frameList->count(); row++)
    {
             if(ui->frameList->item(row)->isSelected()){
                if(do_not_move)
                    continue;
                //move up once
                QListWidgetItem * currentItem = ui->frameList->takeItem(row);
                ui->frameList->insertItem(row - 1, currentItem);
                ui->frameList->item(row-1)->setSelected(true);
             }else{
                 do_not_move = false;
             }
    }
    ui->frameList->setFocus();
}

void MainWindow::on_down_button_clicked()
{
    int do_not_move = true;
    for(int row = ui->frameList->count()-1; row > -1; row--)
    {
             QListWidgetItem *item = ui->frameList->item(row);
             if(item->isSelected()){
                if(do_not_move)
                    continue;
                //move up once
                QListWidgetItem * currentItem = ui->frameList->takeItem(row);
                ui->frameList->insertItem(row + 1, currentItem);
                ui->frameList->item(row+1)->setSelected(true);
             }else{
                 do_not_move = false;
             }
    }
    ui->frameList->setFocus();
}

void MainWindow::loadWatermark()
{
    QString file = QFileDialog::getOpenFileName(
                        this,
                        tr("Select an image to open as watermark"),
                        QString(), // "/home",
                        tr("Images (*.png *.xpm *.jpg *.jpeg *.gif)"));
    loadWatermark(file);
}

void MainWindow::loadWatermark(const QString& watermark)
{
    vw_.setWatermark(watermark);
    currentFrame_.setWatermark(QPixmap(watermark));
    ui->load_watermark->setIcon(QIcon(":/icons/edit-delete.png"));
    setStatusBar("watermark image loaded");
}

void MainWindow::on_load_watermark_clicked()
{
    if(vw_.getWatermark()=="")
        loadWatermark();
    else
        remove_watermark();
}

void  MainWindow::remove_watermark(){
    vw_.setWatermark("");
    currentFrame_.setWatermark(QPixmap(""));
    ui->load_watermark->setIcon(QIcon(":/icons/color-fill.png"));
    setStatusBar("removed watermark image");
}

void MainWindow::updateSlider_(){
       if(ui->frameList->count()!=0)
           ui->frameSlider->setRange(0,ui->frameList->count()-1);
       else{
           ui->frameSlider->setValue(0);
           ui->frameSlider->setRange(0,0);
       }
}

void MainWindow::on_sizeSpinbox_valueChanged(double v)
{
    currentFrame_.changeSize(v/100);
    currentFrame_.update();
}

void MainWindow::on_opacitySpinbox_valueChanged(double v)
{
    currentFrame_.changeOpacity(v/100);
    currentFrame_.update();
}

void MainWindow::on_watermark_x_dial_valueChanged(int value)
{
    currentFrame_.changePosX(value/100.);
    QString tooltip = "watermark position on x-axis: ";
    tooltip += QString::number(value)+"%";
    ui->watermark_x_dial->setToolTip(tooltip);
}

void MainWindow::on_watermark_y_dial_valueChanged(int value)
{
    currentFrame_.changePosY(value/100.);
    QString tooltip = "watermark position on y-axis: ";
    tooltip += QString::number(value)+"%";
    ui->watermark_y_dial->setToolTip(tooltip);
}

void MainWindow::scaleUpWatermark(int steps){
    currentFrame_.scaleSize(steps);
    ui->sizeSpinbox->setValue(currentFrame_.getSize()*100);
}

void MainWindow::changeWatermarkOpacity(int steps){
    currentFrame_.modifyOpacity(steps);
    ui->opacitySpinbox->setValue(currentFrame_.getOpacity()*100);
}

void MainWindow::moveWatermark(float x, float y){
    currentFrame_.changePos(x, y);
    ui->watermark_x_dial->setValue(100* currentFrame_.getPosX());
    ui->watermark_y_dial->setValue(100* currentFrame_.getPosY());
}

void MainWindow::on_actionLoad_Watermark_triggered()
{
    loadWatermark();
}

void MainWindow::on_addImages_button_clicked()
{
    loadImages();
}

void MainWindow::on_actionAbout_triggered()
{
    QString titleText("About");
    QMessageBox::about(this,titleText,
                       "<center><font size=6><b> frameboy </b></font><br>"
                       "<font size=\"3\">"
                       "Version: "
                       GIT_CURRENT_SHA1
                       "&nbsp;&nbsp;&nbsp;&nbsp;build: "
                       __DATE__
                       " "
                       __TIME__
                       "<br>"
                       "<br>"
                       "Combine a sequence of images with a watermark ond render a AVI video."
                       "<br><br>"
                       "Copyright &copy; 2015 kenny@wook.de"
                       "<br><br>"
                       "This program comes with absolutely no warranty."
                       "See the <a href=http://www.gnu.org/licenses/gpl.html>GNU General Public License, version 3</a> for details."
                       "</font></center>");
}

void MainWindow::on_actionHelp_triggered()
{
    QString titleText("Quick Introduction");
    QMessageBox::information(this,titleText,
                           "To create an AVI file from a sequence of images perform the following step. "
                           "No other video formats than AVI(MPG4) available."
                           "<ol>"
                           "<li>Load frames</li>"
                           "<ul>"
                              "<li>Rearange</li>"
                           "</ul>"
                           "<li>Load watermark (optional)</li>"
                             "<ul>"
                             "<li>Select appropriate size, opacity and location.</li>"
                             "</ul>"
                           "<li>Select output</li>"
                           "<ul>"
                           "<li>Click right to \"Target\"</li>"
                           "</ul>"
                           "<li>Write the video.</li>"
                           "<ul>"
                           "<li>Start conversion</li>"
                           "</ul>"
                           "</ol>");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QStringList list;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &name = url.toLocalFile();

        if( name.endsWith(".jpg",Qt::CaseInsensitive) or
            name.endsWith(".jpeg",Qt::CaseInsensitive) or
            name.endsWith(".png",Qt::CaseInsensitive) or
            name.endsWith(".xpm",Qt::CaseInsensitive)){
            list.append(name);
        }
    }
    loadImages(list);
}

void MainWindow::on_top_button_clicked()
{
    int count = ui->frameList->count();
    int pos = count-1;

    while(count-->0)
    {
        if(ui->frameList->item(pos)->isSelected()){
            QListWidgetItem * currentItem = ui->frameList->takeItem(pos);
            ui->frameList->insertItem(0, currentItem);
            ui->frameList->item(0)->setSelected(true);
        }else{
            pos-=1;
        }
    }
    ui->frameList->setFocus();
}

void MainWindow::on_bottom_button_clicked()
{
    int count = ui->frameList->count()-1;
    int pos = 0;

    while(count-->0)
    {
        if(ui->frameList->item(pos)->isSelected()){
            QListWidgetItem * currentItem = ui->frameList->takeItem(pos);
            ui->frameList->insertItem(ui->frameList->count(), currentItem);
            ui->frameList->item(ui->frameList->count()-1)->setSelected(true);
        }else{
            pos+=1;
        }
    }
    ui->frameList->setFocus();
}

void MainWindow::keyPressEvent( QKeyEvent * event ){
    if(event->key() == Qt::Key_Shift){
        ui->graphicsView->setShift(true);
    }else if(event->key() == Qt::Key_Left){//previous frame
        on_frameSlider_valueChanged((ui->frameList->currentRow()-1)%(ui->frameList->count()));
    }else if(event->key() == Qt::Key_Right) {//next frame
        on_frameSlider_valueChanged((ui->frameList->currentRow()+1)% (ui->frameList->count()));
    }
    event->accept();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Shift)
        ui->graphicsView->setShift(true);
    event->accept();
}
