//
//    APTInspectorWidgetFactory.h: Make APT inspector widgets
//    Copyright (C) 2022 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//
#ifndef APTInspectorWidgetFACTORY_H
#define APTInspectorWidgetFACTORY_H

#include <InspectionWidgetFactory.h>

class APTInspectorWidgetFactory : public SigDigger::InspectionWidgetFactory
{
public:
  APTInspectorWidgetFactory(Suscan::Plugin *);

  const char *name() const override;

  SigDigger::InspectionWidget *make(
      Suscan::AnalyzerRequest const &,
      SigDigger::UIMediator *) override;
};

#endif // APTInspectorWidgetFACTORY_H
