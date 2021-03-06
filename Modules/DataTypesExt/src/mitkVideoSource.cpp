/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVideoSource.h"

mitk::VideoSource::VideoSource()
  : m_CurrentVideoTexture(nullptr),
    m_CaptureWidth(0),
    m_CaptureHeight(0),
    m_CapturingInProcess(false),
    m_FrameCount(0),
    m_CapturePaused(false)
{
}

mitk::VideoSource::~VideoSource()
{
  if (m_CurrentVideoTexture)
    delete m_CurrentVideoTexture;
}

void mitk::VideoSource::StartCapturing()
{
  m_CapturingInProcess = true;
  m_FrameCount = 0;
  this->Modified();
}

void mitk::VideoSource::StopCapturing()
{
  m_CapturingInProcess = false;
  m_FrameCount = 0;
  this->Modified();
}

bool mitk::VideoSource::IsCapturingEnabled() const
{
  return m_CapturingInProcess;
}

void mitk::VideoSource::FetchFrame()
{
  ++m_FrameCount;
  this->Modified();
}

int mitk::VideoSource::GetImageWidth()
{
  return m_CaptureWidth;
}

int mitk::VideoSource::GetImageHeight()
{
  return m_CaptureHeight;
}

unsigned long mitk::VideoSource::GetFrameCount() const
{
  return m_FrameCount;
}

bool mitk::VideoSource::GetCapturePaused() const
{
  return m_CapturePaused;
}

void mitk::VideoSource::PauseCapturing()
{
  m_CapturePaused = !m_CapturePaused;
}
