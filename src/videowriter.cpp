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

#include "videowriter.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QImage>
#include <QPen>
#include <QPainter>
#include <QDebug>

#include <thread>
#include <future>

//open cv
#include <cv.h>
#include <highgui.h>

#include "common.h"

bool VideoWriter::createTempDir_() {
    temp_dir_ = QDir::tempPath()+QDir::separator()+"makemovie";
    QDir dir(temp_dir_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.exists();
}

bool VideoWriter::removeDir_(const QString & dirName) const
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir_(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}



bool VideoWriter::writeVideoFromImages_(){

    int CODEC = CV_FOURCC('M','P','4','2');
    // Load input video
    std::string images = (temp_dir_ + QDir::separator()).toUtf8().constData();
    images += "%4d.png";
    cv::VideoCapture input_cap(images);
    if (!input_cap.isOpened())
    {
        emit signalError("Error: Input video could not be opened!");
        return false;
    }

    // Setup output video
    cv::VideoWriter output_cap(filename_.c_str(),
                                CODEC,
                                fps_,
                                cv::Size(input_cap.get(CV_CAP_PROP_FRAME_WIDTH),
                                input_cap.get(CV_CAP_PROP_FRAME_HEIGHT)));

    if (!output_cap.isOpened())
    {
        emit signalError("Error: Output video could not be opened!");
        return false;
    }
    // Loop to read frames from the input capture and write it to the output capture
    cv::Mat frame;
    int i = 0;
    while (true)
    {
        if (!input_cap.read(frame))
            break;
        i+=1;
        QString msg = QString("encoding video ")+ QString::number( 100.*i / images_.size() )+"%";
        emit signalGUI(msg);


        output_cap.write(frame);
    }
    // Release capture interfaces
    input_cap.release();
    output_cap.release();

    return true;
}


void VideoWriter::writeVideo(){

    if(filename_==""){
        emit signalError("Please choose an output filename.");
        emit signalGUI("Please choose an output filename.");
        return;
    }

    if(images_.count()==0){
        emit signalError("Please add some images.");
        emit signalGUI("Please add some images.");
        return;
    }


    bool success = createTempDir_();
    if(!success){
        qDebug()<<"could not create temp dir!\n";
        emit signalError("Could not create a temporary directory to store single frames!");
        emit signalGUI("Could not create a temporary directory to store single frames!");
        removeTempDir_();
        return;
    }

    success = copyImages_();

    if(not success)
        return;

    QString msg = "copied images";
    emit signalGUI(msg);


    success = writeVideoFromImages_();
    removeTempDir_();

    if (not success)
        return;

    msg = "video written: ";
    msg += QString::fromStdString(filename_);
    emit signalGUI(msg);
}


bool VideoWriter::copyImages_(int threads){

    bool use_watermark_ = (watermark_!="");
    QImage watermark;
    if(use_watermark_)
        watermark = QImage(watermark_.c_str());

    unsigned int count = 0;
    for (int i = 0; i < images_.size(); ++i){
        QString file = images_.at(i);
        QString filename = QString::number(count);
        filename = filename.rightJustified(4, '0');
        filename += ".png";
        QString new_file = temp_dir_ + QDir::separator() + filename;
        ++count;

        if(QFile(file).size()==0){
            QString msg = "File "+file+" seems to be invalid!";
            qDebug()<<msg;
            emit signalError(msg);
            emit signalGUI("A file seems to be invalid!");
            removeTempDir_();
            //return false;
        }

        bool image_copied = false;
        if(not use_watermark_ and file.endsWith(".png",Qt::CaseInsensitive)){
            image_copied = QFile::copy(file, new_file);
        }
        if(not image_copied){
            QImage image;
            bool loaded = image.load(file);
            if(not loaded){
                QString msg = "File "+file+" could not be loaded!";
                qDebug()<<msg;
                emit signalError(msg);
                emit signalGUI("Image could not be opened!");
                removeTempDir_();
                //return false;
            }

            if(use_watermark_  and scale_watermark_ >0 ){
                QPainter painter(&image);


                watermark = watermark.scaled(image.size()*scale_watermark_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPointF pos( posX_*.01*(image.width()-watermark.width()),
                             posY_*.01*(image.height()-watermark.height()));
                painter.setOpacity(opacity_);
                painter.drawImage(pos, watermark);
            }
            if(!image.save(new_file)){
                qDebug() << "saving modified image failed!\n";
                qDebug() << file << " -> "<<new_file<<"\n";
                QString msg = "Saving modified file as "+new_file+" failed!";
                emit signalError(msg);
                emit signalGUI("Saving modified file failed!");
                removeTempDir_();
                //return false;
            }
        }
        QString msg = "modifying frames: "+ QString::number(100.*count/images_.size())+"%";
        emit signalGUI(msg);
    }
    return true;
}
