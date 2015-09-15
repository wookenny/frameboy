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

#ifndef VIDEOWRITER_H
#define VIDEOWRITER_H

#include <string>
#include <QDir>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QPair>

class VideoWriter : public  QThread{
Q_OBJECT
    private:
        QString filename_;
        double fps_ = 25.;
        QString sep_ = QDir::separator();
        QString watermark_;
        QImage* watermark_file_ = nullptr;
        float opacity_ = .8;
        float posX_ = .9;
        float posY_ = .9;
        float scale_watermark_ = .1;
        QStringList images_;
        QString temp_dir_;
	    QPair<int,int> resolution_;

        bool createTempDir_();
        bool writeVideoFromImages_();
        bool copyImages_();
        bool removeDir_(const QString &) const;
        bool removeTempDir_() const{ return removeDir_(temp_dir_) ;}
        unsigned int nthreads_ = QThread::idealThreadCount();
    protected:
        void run(){ writeVideo(); }

    signals:
        void signalGUI(const QString &msg);
        void signalError(const QString &msg);

    public:
        void sendSignalGUI(const QString &msg);
        void setOutputFilename(QString s) { filename_ = s; }
        void setFramerate(double fps) { fps_ = fps;}
        void setWatermark(QString w)  { watermark_ = w;}
        void setWatermarkOpacity(float o) { opacity_   = o;}
        void setWatermarkPosX(float x)  { posX_ = x;}
        void setWatermarkPosY(float y)  { posY_ = y;}
        void setWatermarkScale(float s)   { scale_watermark_ = s;}
        void setImages(QStringList l)     { images_ = l; }

        void writeVideo();

        QString getWatermark() const{return watermark_;}

        friend class ImageWriter;
};


class ImageWriter: public QThread
{
public:
    explicit ImageWriter(uint id, VideoWriter *v): threadID_(id), vw_(v){}
    void run();
    QString getErrorMsg() const{return error_;}

private:
    uint threadID_;
    VideoWriter *vw_;
    QString error_;
};

#endif // VIDEOWRITER_H
