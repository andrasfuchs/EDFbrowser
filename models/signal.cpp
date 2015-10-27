#include <QString>
#include <QColor>

#include "signal.h"

Signal::Signal(QString id, QString name, QString alias, QVector<double> values, QString h_name, QString h_unit, double h_density, QString v_name, QString v_unit, double v_density)
{
  this->id = id;
  this->name = name;
  this->alias = alias;

  this->values = values;

  this->h_name = h_name;
  this->h_unit = h_unit;
  this->h_density = h_density;

  this->v_name = v_name;
  this->v_unit = v_unit;
  this->v_density = v_density;
}


QString Signal::GetId()
{
  return this->id;
}

QString Signal::GetName()
{
  return this->name;
}

QString Signal::GetAlias()
{
  return this->alias;
}

void Signal::SetAlias(QString alias)
{
  if (this->alias != alias)
  {
    this->alias = alias;

    emit aliasChanged(this->alias);
  }
}

QVector<double> Signal::GetValues()
{
  return values;
}

void Signal::SetValues(QVector<double> values)
{
  if (this->values != values)
  {
      this->values = values;

      emit valuesChanged(this->values);
  }
}

QColor Signal::GetColor()
{
  return color;
}

void Signal::SetColor(QColor color)
{
  if (this->color != color)
  {
      this->color = color;

      emit colorChanged(this->color);
  }
}

double Signal::GetPosition()
{
  return position;
}

void Signal::SetPosition(double position)
{
    if (position < 0)
    {
        position = 0;
    }

    if (position > values.count()-1)
    {
        position = values.count()-1;
    }

  if (this->position != position)
  {
      this->position = position;

      emit positionChanged(this->position);
  }
}

void Signal::GetSubSignal(SubSignalType type, double parameter)
{
    // TODO: implement this method
  return;
}

double Signal::GetHorizontalDensity()
{
    return h_density;
}
