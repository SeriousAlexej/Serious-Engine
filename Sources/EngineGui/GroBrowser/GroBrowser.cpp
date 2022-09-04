/* Copyright (c) 2022 SeriousAlexej (Oleksii Sierov).
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

#include "stdh.h"
#include "GroBrowser.h"
#include "GroBrowser.ui.h"

GroBrowser::GroBrowser(QWidget* parent)
  : QDialog(parent)
  , mp_ui(std::make_unique<Ui::GroBrowser>())
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  mp_ui->setupUi(this);

  connect(mp_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mp_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

GroBrowser::~GroBrowser()
{
}
