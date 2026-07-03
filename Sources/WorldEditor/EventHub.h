/* Copyright (c) 2021 SeriousAlexej (Oleksii Sierov).
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */
#ifndef EVENT_HUB_H
#define EVENT_HUB_H

#include "Tree/base_property_tree_item.h"

#include <vector>

struct CEntity_;
Q_DECLARE_OPAQUE_POINTER(CEntity_*)
Q_DECLARE_METATYPE(CEntity_*)

class EventHub : public QObject
{
  Q_OBJECT
public:
  static EventHub& instance();

  Q_SIGNAL void CurrentEntitySelectionChanged(const std::set<CEntity_*>&);
  Q_SIGNAL void PropertyChanged(const std::set<CEntity_*>&, CPropertyID*, BasePropertyTreeItem*);
  Q_SIGNAL void EntityPicked(CEntity_*);
  Q_SIGNAL void SelectionStealerInstalled(void*);
};

#endif
