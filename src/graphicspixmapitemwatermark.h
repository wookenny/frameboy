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

#ifndef GRAPHICSPIXMAPITEMWATERMARK_H
#define GRAPHICSPIXMAPITEMWATERMARK_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QDebug>

class GraphicsPixmapItemWatermark : public QGraphicsPixmapItem
{

public:
    GraphicsPixmapItemWatermark(QGraphicsItem *parent=nullptr): QGraphicsPixmapItem(parent){
        show_watermark_ = true;
        opacity_ = .5;
        size_ = .1;
        posx_ = .9;
        posy_ = .9;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setWatermark(QPixmap w){watermark_ = w;
                                 //TODO: BAD!!!!
                                 this->hide(); this->show();}
    void changeOpacity(float value);
    void changeSize(float value){ size_ = value; }
    void scaleSize(int steps){  if(watermark_.isNull()) return;
                                size_ += steps*.01;
                                size_ = std::min(1.f,size_);
                                size_ = std::max(0.f,size_);
                                this->hide(); this->show();
                                //TODO: BAD!!!!
                              }
    void modifyOpacity(int steps){  if(watermark_.isNull()) return;
                                opacity_ += steps*.01;
                                opacity_ = std::min(1.f,opacity_);
                                opacity_ = std::max(0.f,opacity_);
                                this->hide(); this->show();
                                //TODO: BAD!!!!
                              }
    float getSize(){return size_;}
    float getOpacity(){return opacity_;}
    float getPosX(){return posx_;}
    float getPosY(){return posy_;}

    void changePos(float x, float y);
    void changePosX(float value){ posx_ = value; this->hide(); this->show();}
    void changePosY(float value){ posy_ = value; this->hide(); this->show();}

private:
    bool show_watermark_;
    float opacity_;
    float size_;
    float posx_;
    float posy_;
    QPixmap watermark_;

};

#endif // GRAPHICSPIXMAPITEMWATERMARK_H
