//
//    APTInspectorWidgetFactory.cpp: Make APT inspectors
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
#include "APTInspectorWidgetFactory.h"
#include "APTInspectorWidget.h"

APTInspectorWidgetFactory::APTInspectorWidgetFactory(Suscan::Plugin *plugin) :
  InspectionWidgetFactory(plugin)
{

}

const char *
APTInspectorWidgetFactory::name() const
{
  return "APT inspector";
}


SigDigger::InspectionWidget *
APTInspectorWidgetFactory::make(
    Suscan::AnalyzerRequest const &request,
    SigDigger::UIMediator *mediator)
{
  return new APTInspectorWidget(this, request, mediator, nullptr);
}
