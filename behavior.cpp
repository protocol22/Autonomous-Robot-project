/*
 * Copyright (C) 2018 Ola Benderius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "behavior.hpp"
#include <cmath>
#include <iostream>

Behavior::Behavior() noexcept:
  m_frontUltrasonicReading{},
  m_rearUltrasonicReading{},
  m_leftIrReading{},
  m_rightIrReading{},
  m_groundSteeringAngleRequest{},
  m_pedalPositionRequest{},
  m_frontUltrasonicReadingMutex{},
  m_rearUltrasonicReadingMutex{},
  m_leftIrReadingMutex{},
  m_rightIrReadingMutex{},
  m_groundSteeringAngleRequestMutex{},
  m_pedalPositionRequestMutex{},
  m_rightDistancePrev{},
  m_frontDistancePrev{}
{
}

opendlv::proxy::GroundSteeringRequest Behavior::getGroundSteeringAngle() noexcept
{
  std::lock_guard<std::mutex> lock(m_groundSteeringAngleRequestMutex);
  return m_groundSteeringAngleRequest;
}

opendlv::proxy::PedalPositionRequest Behavior::getPedalPositionRequest() noexcept
{
  std::lock_guard<std::mutex> lock(m_pedalPositionRequestMutex);
  return m_pedalPositionRequest;
}

void Behavior::setFrontUltrasonic(opendlv::proxy::DistanceReading const &frontUltrasonicReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_frontUltrasonicReadingMutex);
  m_frontUltrasonicReading = frontUltrasonicReading;
}

void Behavior::setRearUltrasonic(opendlv::proxy::DistanceReading const &rearUltrasonicReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_rearUltrasonicReadingMutex);
  m_rearUltrasonicReading = rearUltrasonicReading;
}

void Behavior::setLeftIr(opendlv::proxy::VoltageReading const &leftIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_leftIrReadingMutex);
  m_leftIrReading = leftIrReading;
}

void Behavior::setRightIr(opendlv::proxy::VoltageReading const &rightIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_rightIrReadingMutex);
  m_rightIrReading = rightIrReading;
}


void Behavior::step() noexcept
{
  opendlv::proxy::DistanceReading frontUltrasonicReading;
  opendlv::proxy::DistanceReading rearUltrasonicReading;
  opendlv::proxy::VoltageReading leftIrReading;
  opendlv::proxy::VoltageReading rightIrReading;
  {
    std::lock_guard<std::mutex> lock1(m_frontUltrasonicReadingMutex);
    std::lock_guard<std::mutex> lock2(m_rearUltrasonicReadingMutex);
    std::lock_guard<std::mutex> lock3(m_leftIrReadingMutex);
    std::lock_guard<std::mutex> lock4(m_rightIrReadingMutex);

    frontUltrasonicReading = m_frontUltrasonicReading;
    rearUltrasonicReading = m_rearUltrasonicReading;
    leftIrReading = m_leftIrReading;
    rightIrReading = m_rightIrReading;
  }

  float frontDistance = frontUltrasonicReading.distance();
  //float rearDistance = rearUltrasonicReading.distance();
  //double leftDistance = convertIrVoltageToDistance(leftIrReading.voltage());
  double rightDistance = convertIrVoltageToDistance(rightIrReading.voltage());
  //double frontDistanceDiff = m_frontDistancePrev -frontDistance;
  double rightDistanceDiff = m_rightDistancePrev - rightDistance;
  m_rightDistancePrev = rightDistance;
  //m_frontDistancePrev = frontDistance;
  float pedalPosition = 0.05f;
  float groundSteeringAngle = 0.0f;

  if(frontDistance <= 0.3f || rightDistanceDiff >0)
  {
    /*if(frontDistance <=0.3f)
      {  pedalPosition = 0.1f;
        groundSteeringAngle = 1.2f;
      }
    else*/
    {
    pedalPosition = 0.03f;
    groundSteeringAngle = 0.6f;
    }

  }

  /*else if(frontDistanceDiff > 0 && rightDistanceDiff > 0)
  { m_frontDistancePrev =0;
    pedalPosition = 0.1f;
    groundSteeringAngle = 0.6f;
  }*/

  else if(rightDistanceDiff < 0 && rightDistance > 0.2f)
  {
    m_rightDistancePrev = 0;
    pedalPosition = 0.01f;
    groundSteeringAngle = -1;
  }
  else
  {
    pedalPosition = 0.04f;
    groundSteeringAngle = 0;
  }

  std::cout << "dist " << rightDistance << std::endl;
  std::cout << "diff " << rightDistanceDiff << std::endl;
  std::cout << "fdist " << frontDistance << std::endl;
  std::cout << "fdiff " << frontDistanceDiff << std::endl;


  /*
  if (frontDistance < 0.3f) {
    pedalPosition = 0.0f;
  } else {
    if (rearDistance < 0.3f) {
      pedalPosition = 0.4f;
    }
  }

  if (leftDistance < rightDistance) {
    if (leftDistance < 0.2f) {
      groundSteeringAngle = 0.2f;
    }
  } else {
    if (rightDistance < 0.2f) {
      groundSteeringAngle = 0.2f;
    }
  }
  */

  {
    std::lock_guard<std::mutex> lock1(m_groundSteeringAngleRequestMutex);
    std::lock_guard<std::mutex> lock2(m_pedalPositionRequestMutex);

    opendlv::proxy::GroundSteeringRequest groundSteeringAngleRequest;
    groundSteeringAngleRequest.groundSteering(groundSteeringAngle);
    m_groundSteeringAngleRequest = groundSteeringAngleRequest;

    opendlv::proxy::PedalPositionRequest pedalPositionRequest;
    pedalPositionRequest.position(pedalPosition);
    m_pedalPositionRequest = pedalPositionRequest;
  }
}

// TODO: This is a rough estimate, improve by looking into the sensor specifications.
double Behavior::convertIrVoltageToDistance(float voltage) const noexcept
{
  double voltageDividerR1 = 1000.0;
  double voltageDividerR2 = 1000.0;

  double sensorVoltage = (voltageDividerR1 + voltageDividerR2) / voltageDividerR2 * voltage;
  double distance = (0.0359*pow(sensorVoltage,4))-(0.2865*pow(sensorVoltage,3))+(0.83788*pow(sensorVoltage,2))-(1.1055*sensorVoltage)+(0.6436);
  return distance;
}