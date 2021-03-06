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

#include "graphicsviewscaling.h"
#include "mainwindow.h"
#include <QDebug>
#include <QTransform>
#include <QResizeEvent>
#include <QUrl>
#include <QMimeData>

GraphicsViewScaling::GraphicsViewScaling(QWidget *parent) :  QGraphicsView(parent)
{
    this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    this->setAcceptDrops(true);
}

void GraphicsViewScaling::setGraphicsSize(int w,int h){
        sizeX_= w; sizeY_ = h;
        updateItemSize();
}

void GraphicsViewScaling::updateItemSize(){
    setSceneRect(0, 0, sizeX_, sizeY_);
    fitInView(this->sceneRect(),Qt::KeepAspectRatio);
}

void GraphicsViewScaling::resizeEvent(QResizeEvent *event)
{
    updateItemSize();
    QGraphicsView::resizeEvent(event);
    event->accept();
}

void GraphicsViewScaling::mousePressEvent ( QMouseEvent * event){

    QPointF p = this->mapToScene(QPoint(event->x(),event->y()));
    event->accept();
    emit signalMoveTo(p.x(),p.y());
}

void GraphicsViewScaling::mouseMoveEvent(QMouseEvent * event)
{
    QPointF p = this->mapToScene(QPoint(event->x(),event->y()));
    event->accept();
    emit signalMoveTo(p.x(),p.y());
}

void GraphicsViewScaling::wheelEvent(QWheelEvent *event){
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    event->accept();
    if(not shift_key_)
        emit signalScaleUp(numSteps);
    else
        emit signalOpacityChange(numSteps);
}

void GraphicsViewScaling::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void GraphicsViewScaling::dropEvent(QDropEvent *e)
{
    //single file => watermark
    if (e->mimeData()->urls().size() == 1){
        const QString &fileName = e->mimeData()->urls().front().toLocalFile();
        if ( fileName.endsWith(".jpg",Qt::CaseInsensitive) or
             fileName.endsWith(".jpeg",Qt::CaseInsensitive) or
             fileName.endsWith(".png",Qt::CaseInsensitive) or
             fileName.endsWith(".gif",Qt::CaseInsensitive) or
             fileName.endsWith(".xpm",Qt::CaseInsensitive) ){
            this->main->loadWatermark(fileName);
            return;
        }
    }else{ //multiple file => frames
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
        this->main->loadImages(list);
    }
}


void GraphicsViewScaling::dragMoveEvent(QDragMoveEvent * e){
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void GraphicsViewScaling::dragLeaveEvent(QDragLeaveEvent *e){
        e->accept();
}

void GraphicsViewScaling::keyPressEvent( QKeyEvent * event ){
    if(event->key() == Qt::Key_Shift){
        shift_key_ = true;
        event->accept();
    }
}

void GraphicsViewScaling::keyReleaseEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Shift){
        shift_key_ = false;
        event->accept();
    }
}
