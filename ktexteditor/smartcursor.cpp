/* This file is part of the KDE project
   Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "smartcursor.h"

#include "document.h"
#include "smartrange.h"

using namespace KTextEditor;

SmartCursor::~ SmartCursor( )
{
}

SmartCursor::SmartCursor( const Cursor & position, Document * doc, bool moveOnInsert )
  : Cursor(position)
  , m_doc(doc)
  , m_moveOnInsert(moveOnInsert)
{
  Q_ASSERT(m_doc);
}

bool SmartCursor::atEndOfDocument( ) const
{
  return *this >= m_doc->documentEnd();
}

bool SmartCursor::isSmart( ) const
{
  return true;
}

bool SmartCursor::insertText( const QStringList & text, bool block )
{
  return document()->insertText(*this, text, block);
}

QChar SmartCursor::character( ) const
{
  return document()->character(*this);
}

SmartRange * SmartCursor::smartRange( ) const
{
  return static_cast<SmartRange*>(m_range);
}

void SmartCursor::setRange( SmartRange * range )
{
  Cursor::setRange(range);
  checkFeedback();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
