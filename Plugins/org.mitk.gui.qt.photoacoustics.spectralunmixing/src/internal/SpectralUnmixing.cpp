/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "SpectralUnmixing.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

// to perform Spectral Unmixing
#include "mitkPASpectralUnmixingFilter.h"

const std::string SpectralUnmixing::VIEW_ID = "org.mitk.views.spectralunmixing";

void SpectralUnmixing::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void SpectralUnmixing::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &SpectralUnmixing::DoImageProcessing);
  connect(m_Controls.ButtonAddWavelength, &QPushButton::clicked, this, &SpectralUnmixing::Wavelength);
}

// Add Wavelength is working, BUT in the Plugin! Not with same implementation at the filter
// probably because the m_wavelengths vector is created new every time and not 'saved'.
// Alternativ as comment here and at the filter.
void SpectralUnmixing::Wavelength()
{
  if (m_Wavelengths.empty())
  {
      size = 0;
  }

  wavelength = m_Controls.spinBoxAddWavelength->value();
  m_Wavelengths.push_back(wavelength);
  MITK_INFO << "ADD WAVELENGTH: " << wavelength << "nm";
  size += 1; // size implemented like this because '.size' is const
  MITK_INFO << "ALL WAVELENGTHS: ";

  for (int i = 0; i < size; ++i)
  {
    MITK_INFO << m_Wavelengths[i] << "nm";
  }
}

// Not correct working alternativ for wavelengths
/*void SpectralUnmixing::Wavelength()
{
  auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilter::New();
  wavelength = m_Controls.spinBoxAddWavelength->value();
  m_SpectralUnmixingFilter->AddWavelength(wavelength);
  MITK_INFO << wavelength << " nm";
}*/

void SpectralUnmixing::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
    {
      m_Controls.labelWarning->setVisible(false);
      m_Controls.buttonPerformImageProcessing->setEnabled(true);
      return;
    }
  }

  m_Controls.labelWarning->setVisible(true);
  m_Controls.buttonPerformImageProcessing->setEnabled(false);
}

void SpectralUnmixing::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;

  mitk::DataNode *node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(nullptr, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "PERFORMING SPECTRAL UNMIXING (SOON)";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // actually do something here...

      // Checking which chromophores wanted for SU if none throw exeption!
      numberofChromophores = 0;
      DeOxbool = m_Controls.checkBoxDeOx->isChecked();
      Oxbool = m_Controls.checkBoxOx->isChecked();
      if (DeOxbool || Oxbool == true)
      {
        MITK_INFO << "CHOSEN CHROMOPHORES:";
      }
      if (Oxbool == true)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Oxyhemoglobin";
      }
      if (DeOxbool == true)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Deoxygenated hemoglobin";
      }
      if (numberofChromophores == 0)
      {
        mitkThrow() << "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!";       
      }

      // Checking if number of wavelengths >= number of chromophores
      if (numberofChromophores > size)
      {
        mitkThrow() << "PRESS 'IGNORE' AND ADD MORE WAVELENGTHS!";
      }
      
    


    }
  }
}
