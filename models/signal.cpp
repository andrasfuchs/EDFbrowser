#include "signal.h"

Signal::Signal(QString id, QString name, QString alias, QVector<double> values, QString h_name, QString h_unit, double h_density, QString v_name, QString v_unit, double v_density, SignalType type)
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

  this->type = type;
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

QString Signal::GetDisplayName()
{
    if (this->GetAlias() != "")
    {
      return this->GetAlias();
    } else {
      return this->GetName();
    }
}


SignalType Signal::GetType()
{
  return this->type;
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

QPen Signal::GetPen()
{
  return pen;
}

void Signal::SetPen(QPen pen)
{
  if (this->pen != pen)
  {
      this->pen = pen;

      emit penChanged(this->pen);
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

void Signal::GetSubSignal(SignalType type, double parameter)
{
    // TODO: implement this method
  return;
}

QString Signal::GetHorizontalName()
{
    return h_name;
}

QString Signal::GetHorizontalUnit()
{
    return h_unit;
}

double Signal::GetHorizontalDensity()
{
    return h_density;
}

void Signal::SetHorizontalDensity(double h_density)
{
    if (this->h_density != h_density)
    {
        this->h_density = h_density;

        emit horizontalDensityChanged(this->h_density);
    }
}

QString Signal::GetVerticalName()
{
    return v_name;
}

QString Signal::GetVerticalUnit()
{
    return v_unit;
}

double Signal::GetVerticalDensity()
{
    return v_density;
}
