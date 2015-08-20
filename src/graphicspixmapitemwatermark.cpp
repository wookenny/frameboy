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

#include "graphicspixmapitemwatermark.h"
#include <QDebug>

void GraphicsPixmapItemWatermark::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter,option,widget);

    //draw watermark if desired
    if(show_watermark_){
        QPen pen(QColor(255,0,0));
        pen.setWidth(1);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QColor(255,0,0,255*opacity_));

        float scalingX = size_*this->boundingRect().width()/watermark_.width();
        float scalingY = size_*this->boundingRect().height()/watermark_.height();

        float scaling = std::min(scalingX,scalingY);
        int sideX = watermark_.width()*scaling;
        int sideY = watermark_.height()*scaling;
        int posx = (this->boundingRect().width()-sideX)  * posx_;
        int posy = (this->boundingRect().height()-sideY) * posy_;
        painter->setOpacity(opacity_);
        if(!watermark_.isNull())
            painter->drawPixmap(posx,posy,sideX,sideY,watermark_);

    }

}

void GraphicsPixmapItemWatermark::changeOpacity(float value){
    opacity_ = value;
}

void GraphicsPixmapItemWatermark::changePos(float x, float y){

    x = x/this->boundingRect().width();
    y = y/this->boundingRect().height();

    //map to matching size
    float scalingX = size_*this->boundingRect().width()/watermark_.width();
    float scalingY = size_*this->boundingRect().height()/watermark_.height();
    float scaling = std::min(scalingX,scalingY);
    int sideX = watermark_.width()*scaling;
    int sideY = watermark_.height()*scaling;


    x -= .5*sideX/(this->boundingRect().width()-sideX);
    y -= .5*sideY/(this->boundingRect().height()-sideY);

    float maxPosX = (this->boundingRect().width()-sideX)/this->boundingRect().width();
    float maxPosY = (this->boundingRect().height()-sideY)/this->boundingRect().height();
    x = std::min(x,maxPosX);
    x = std::max(0.f,x);
    y = std::max(0.f,y);
    y = std::min(y,maxPosY);

    x = x/(maxPosX);
    y = y/(maxPosY);

    //set midpoint to cursor if possible
    changePosX(x);
    changePosY(y);

}
