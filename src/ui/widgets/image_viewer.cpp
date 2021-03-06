/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/widgets/image_viewer.h"

#include <DWidgetUtil>
#include <DThemeManager>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPoint>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QtCore/QTimer>
#include <QDesktopWidget>
#include <QLabel>

namespace dstore {

namespace {

const int kBorderSize = 12;
const int kBtnSize = 48;

}  // namespace

ImageViewer::ImageViewer(QWidget* parent)
    : QDialog(parent) {
  this->setObjectName("ImageViewer");
  this->initUI();
  this->initConnection();
}

ImageViewer::~ImageViewer() {

}

void ImageViewer::open(const QString& filepath) {
  // Escape uri.
  QString abspath(filepath);
  if (abspath.contains("://")) {
    QUrl url(abspath);
    abspath = url.path();
  }

  const QPixmap pixmap(abspath);
  this->openPixmap(pixmap);
}

void ImageViewer::openPixmap(QPixmap pixmap) {
  if (!this->isVisible()) {
    return;
  }
  const QRect screen_rect = qApp->desktop()->screenGeometry(QCursor::pos());
  const auto ratio=qApp->devicePixelRatio();
  pixmap.setDevicePixelRatio(ratio);
  const int pixmap_max_width = static_cast<int>(screen_rect.width() * 0.8);
  const int pixmap_max_height = static_cast<int>(screen_rect.height() * 0.8);
  if ((pixmap.width() > pixmap_max_width) ||
      (pixmap.height() > pixmap_max_height)) {
    pixmap = pixmap.scaled(pixmap_max_width,
                           pixmap_max_height,
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
  }

  this->move(screen_rect.topLeft());
  this->resize(screen_rect.size());
  this->showFullScreen();

  // Hide spinner.
  spinner_->hide();
  spinner_->stop();

  // Update pixmap of image label.
  img_label_->setPixmap(pixmap);
  img_label_->setFixedSize(pixmap.width()/ratio, pixmap.height()/ratio);
  QRect img_rect = img_label_->rect();
  img_rect.moveTo(
      ((screen_rect.width() - pixmap.width()/ratio) / 2.0),
      ((screen_rect.height() - pixmap.height()/ratio) / 2.0));
  img_label_->move(img_rect.topLeft());

  // Move close button to top-right corner of image.
  const QPoint top_right_point = img_rect.topRight();
  close_button_->move(top_right_point.x() - kBtnSize / 2,
                      top_right_point.y() - kBtnSize / 2);
  close_button_->show();
  close_button_->raise();
}

void ImageViewer::showIndicator() {
  const QRect screen_rect = qApp->desktop()->screenGeometry(QCursor::pos());
  this->move(screen_rect.topLeft());
  this->resize(screen_rect.size());
  this->showFullScreen();

  // Show spinner, moving to center of screen.
  spinner_->show();
  spinner_->start();
  spinner_->move(screen_rect.center());

  // Reset pixmap of image label
  const QSize size = img_label_->size();
  QPixmap empty_pixmap(size);
  empty_pixmap.fill(QColor(0, 0, 0, 1));
  img_label_->setPixmap(empty_pixmap);

  // Hide close button
  close_button_->hide();
}

void ImageViewer::initConnection() {
  connect(close_button_, &Dtk::Widget::DIconButton::clicked,
          this, &ImageViewer::close);
  connect(previous_button_, &Dtk::Widget::DIconButton::clicked,
          this, &ImageViewer::previousImageRequested);
  connect(next_button_, &Dtk::Widget::DIconButton::clicked,
          this, &ImageViewer::nextImageRequested);

  connect(close_button_, &DIconButton::pressed,
          this, [&]() {

      close_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/close_press.svg"));

  });
  connect(close_button_, &DIconButton::released,
          this, [&]() {

      close_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/close_normal.svg"));

  });


  connect(previous_button_, &DIconButton::pressed,
          this, [&]() {

      previous_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/previous_press.svg"));

  });
  connect(previous_button_, &DIconButton::released,
          this, [&]() {

      previous_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/previous_normal.svg"));

  });


  connect(next_button_, &DIconButton::pressed,
          this, [&]() {

      next_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/next_press.svg"));

  });
  connect(next_button_, &DIconButton::released,
          this, [&]() {

      next_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/next_normal.svg"));

  });
}

void ImageViewer::initUI() {
  img_label_ = new QLabel();
  img_label_->setObjectName("ImageLabel");
  img_label_->setMinimumSize(640, 480);

  QSize icon_size(kBtnSize,kBtnSize);
  close_button_ = new DIconButton(this);
  close_button_->setFixedSize(kBtnSize,kBtnSize);
  close_button_->setIconSize(icon_size);
  close_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/close_normal.svg"));
  close_button_->setFlat(true);
  close_button_->raise();

  previous_button_ = new DIconButton(this);
  previous_button_->setFixedSize(kBtnSize,kBtnSize);
  previous_button_->setIconSize(icon_size);
  previous_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/previous_normal.svg"));
  previous_button_->setFlat(true);
  previous_button_->raise();
  previous_button_->setShortcut(Qt::Key_Left);

  next_button_ = new DIconButton(this);
  next_button_->setFixedSize(kBtnSize,kBtnSize);
  next_button_->setIconSize(icon_size);
  next_button_->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/next_normal.svg"));
  next_button_->setFlat(true);
  next_button_->raise();
  next_button_->setShortcut(Qt::Key_Right);

  spinner_ = new Dtk::Widget::DSpinner(this);
  spinner_->setFixedSize(96, 96);


  const QRect screen_rect = qApp->desktop()->screenGeometry(QCursor::pos());
  auto margin = screen_rect.width()*4/100;
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(10);
  layout->addWidget(previous_button_);
  layout->addWidget(img_label_, 1, Qt::AlignCenter);
  layout->addWidget(next_button_);

  this->setContentsMargins(margin, 0, margin, 0);
  this->setLayout(layout);
  this->setWindowFlags(Qt::FramelessWindowHint |
                       Qt::BypassWindowManagerHint |
                       Qt::Dialog |
                       Qt::WindowStaysOnTopHint);
  this->setAttribute(Qt::WA_TranslucentBackground, true);
  this->setModal(true);

  Dtk::Widget::DThemeManager::instance()->registerWidget(this);
}

void ImageViewer::mousePressEvent(QMouseEvent* event) {
  QWidget::mousePressEvent(event);
  this->hide();
}

void ImageViewer::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.fillRect(0, 0, this->width(), this->height(), QColor(0, 0, 0, 77));
}

}  // namespace dstore
