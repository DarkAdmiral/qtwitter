/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef ENTRY_H
#define ENTRY_H

#include <QMetaType>
#include <QDateTime>
#include "twitterapi_global.h"

class QString;

struct TWITTERAPI_EXPORT Entry
{
  enum Type {
    Status,
    DirectMessage
  };

  Entry( Entry::Type entryType = Entry::Status );

  void initialize();
  bool checkContents();

  Type type;
  bool isOwn;
  int id;
  QString text;
  QString originalText;
  QString name;
  QString login;
  QString image;
  QString homepage;
  bool hasHomepage;
  QDateTime timestamp;
};

Q_DECLARE_METATYPE(Entry)

#endif //ENTRY_H
