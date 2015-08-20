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

#ifndef GRAPHICSVIEWSCALING_H
#define GRAPHICSVIEWSCALING_H

#include <QGraphicsView>
#include <QMouseEvent>


class GraphicsViewScaling : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsViewScaling(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent ( QMouseEvent * even);
    void mouseMoveEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent *event);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void dragMoveEvent(QDragMoveEvent * event);
    //void dragDropEvent

signals:
    void signalScaleUp(int steps);
    void signalMoveTo(float x,float y);
private slots:


private:

};

#endif // GRAPHICSVIEWSCALING_H
