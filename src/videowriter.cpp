
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
#include <QElapsedTimer>
#include <QHash>


//open cv
#include <cv.h>
#include <highgui.h>

#include "common.h"

void VideoWriter::sendSignalGUI(const QString &msg){
    emit signalGUI(msg);
}

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
    images += "%6d.jpg";
    cv::VideoCapture input_cap(images);
    if (!input_cap.isOpened())
    {
        emit signalError("Error: Input video could not be opened!");
        return false;
    }

    // Setup output video
    cv::VideoWriter output_cap(filename_.toUtf8().constData(),
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
        QString msg = QString("encoding video ")+ QString::number( 100.*i / images_.size(),'f',3 )+"%";
        emit signalGUI(msg);


        output_cap.write(frame);
    }
    // Release capture interfaces
    input_cap.release();
    output_cap.release();

    return true;
}


void VideoWriter::writeVideo(){
    QElapsedTimer timer;
    timer.start();

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

    float ms = timer.elapsed()/1000.;
    msg = "video written in ";
    msg += QString::number(ms);
    msg += " seconds: ";
    msg += filename_;
    emit signalGUI(msg);
}


bool VideoWriter::copyImages_(){
    bool use_watermark_ = (watermark_!="");
    QImage watermark;

    if(use_watermark_){
        watermark = QImage(watermark_);
        watermark_file_ = &watermark;
    }

    //store the frequency of framesizes
    QHash<QPair<int,int>, int> frame_sizes;
    for(int i=0; i<images_.size(); ++i){
        QImage img;
        bool loaded = img.load(images_.at(i));
        if( not loaded)
        continue;
        //count the frameresolution
        QPair<int,int> resolution(img.width(),img.height());
        if(frame_sizes.find(resolution)==frame_sizes.end())
        frame_sizes[resolution]=1;
        else
        frame_sizes[resolution]+=1;

    }
    int max_count = 0;
    QHash<QPair<int,int>, int>::iterator iter;
    for (iter = frame_sizes.begin(); iter != frame_sizes.end(); ++iter){
        if (iter.value()> max_count){
            max_count = iter.value();
            resolution_ = iter.key();
        }
    }

    //create threads and start them
    std::vector<ImageWriter*> threads;
    for(uint i=0; i<nthreads_;++i)
        threads.push_back(new ImageWriter( i, this) );
    for(uint i=0; i<threads.size();++i)
        threads[i]->start();
    for(uint i=0; i<threads.size();++i)
        threads[i]->wait();
    //collect possible errors
    for(uint i=0; i<threads.size();++i){
        QString error = threads[i]->getErrorMsg();
        if( error !=""){
            signalError(error);
            break;
        }
    }
    //delete all threads
    for(uint i=0; i<threads.size();++i)
        delete threads[i];

    return true;
}


void ImageWriter::run()
{
    error_ = "";
    for (int i = threadID_; i < vw_->images_.size(); i+=vw_->nthreads_){
        QString file = vw_->images_.at(i);
        QString filename = QString::number(i);
        filename = filename.rightJustified(6, '0');
        filename += ".jpg";
        QString new_file = vw_->temp_dir_ + QDir::separator() + filename;

        if(QFile(file).size()==0){
            error_ = "File "+file+" seems to be invalid!";
            qDebug()<<error_;
            return;
        }

        bool image_copied = false;
        if(vw_->watermark_file_== nullptr and file.endsWith(".jpg",Qt::CaseInsensitive)){
            QImage img(file);
            if (not file.isNull() and vw_->resolution_.first == img.width()
                                  and vw_->resolution_.second == img.height() )
                                  image_copied = QFile::copy(file, new_file);
        }
        if(not image_copied){
            QImage image;
            bool loaded = image.load(file);
            if(not loaded){
                error_ = "File "+file+" could not be loaded!";
                qDebug()<<error_;
                return;
            }

            //scale image to desired size
            image = image.scaled(vw_->resolution_.first, vw_->resolution_.second,
                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            if( vw_->watermark_file_ != nullptr){
                QPainter painter(&image);
                QImage wm = *(vw_->watermark_file_);
                wm = wm.scaled(image.size()*vw_->scale_watermark_, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                QPointF pos( vw_->posX_*.01*(image.width()-wm.width()),
                             vw_->posY_*.01*(image.height()-wm.height()));
                painter.setOpacity(vw_->opacity_);
                painter.drawImage(pos, wm);
            }
            if(!image.save(new_file,"jpg",100)){
                error_ = "could not save modified image" + new_file;
                qDebug() << "saving modified image failed!\n";
                qDebug() << file << " -> "<<new_file<<"\n";
                return;
            }
        }
        if(threadID_==0){
            QString msg = "modifying frames: "+ QString::number(100.*i/vw_->images_.size(),'f',3)+"%";
            vw_->sendSignalGUI(msg);
        }
    }
}
