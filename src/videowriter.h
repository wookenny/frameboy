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

class VideoWriter : public QObject{
Q_OBJECT
    private:
        std::string filename_;
        double fps_ = 25.;
        std::string sep_ = ""+ QDir::separator().unicode();
        std::string watermark_;
        float opacity_ = .8;
        float posX_ = .9;
        float posY_ = .9;
        float scale_watermark_ = .1;
        QStringList images_;
        QString temp_dir_;

        bool createTempDir_();
        bool writeVideoFromImages_();
        bool copyImages_();
        bool removeDir_(const QString &) const;
        bool removeTempDir_() const{ return removeDir_(temp_dir_) ;}
        unsigned int nthreads_ = QThread::idealThreadCount();

    signals:
        void signalGUI(const QString &msg);
        void signalError(const QString &msg);

    public:
        void sendSignalGUI(const QString &msg);
        void setOutputFilename(std::string s) { filename_ = s; }
        void setFramerate(double fps) { fps_ = fps;}
        void setWatermark(std::string w)  { watermark_ = w;}
        void setWatermarkOpacity(float o) { opacity_   = o;}
        void setWatermarkPosX(float x)  { posX_ = x;}
        void setWatermarkPosY(float y)  { posY_ = y;}
        void setWatermarkScale(float s)   { scale_watermark_ = s;}
        void setImages(QStringList l)     { images_ = l; }

        void writeVideo();

};


class VideoWriterThread : public QThread
{
Q_OBJECT
public:

    void setWriter(VideoWriter *w){vw_ = w;}

//signals:
//    void signalGUI(const QString &msg);

private:
    VideoWriter *vw_;
protected:
    void run()
        {
            vw_->writeVideo();
        }
};



class ImageWriter: public QThread
{
public:
    explicit ImageWriter(const QImage* w,const QString& d, QStringList &i,uint id, uint n, float s, float x, float y, float o, VideoWriter* v):
                watermark_(w), temp_dir_(d), images_(i), threadID_(id), numThreads_(n),scaleWatermark_(s), posx_(x), posy_(y), opacityWatermark_(o),vw_(v){}

    void run();


private:
    const QImage *watermark_;
    const QString& temp_dir_;
    QStringList &images_;
    uint threadID_;
    uint numThreads_;
    float scaleWatermark_;
    float posx_; float posy_;
    float opacityWatermark_;
    VideoWriter* vw_;
};
#endif // VIDEOWRITER_H
