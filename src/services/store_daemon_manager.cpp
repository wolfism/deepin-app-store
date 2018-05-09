/*
 * Copyright (C) 2018 Deepin Technology Co., Ltd.
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

#include "services/store_daemon_manager.h"

namespace dstore {

StoreDaemonManager::StoreDaemonManager(QObject* parent)
    : QObject(parent),
      apps_() {
  this->setObjectName("StoreDaemonManager");
}

StoreDaemonManager::~StoreDaemonManager() {

}

void StoreDaemonManager::updateAppList(const AppSearchRecordList& app_list) {
  apps_.clear();
  for (const AppSearchRecord& app : app_list) {
    apps_.insert(app.name, app);
  }
}

}  // namespace dstore