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

#ifndef __mitkTbssImporter_h
#define __mitkTbssImporter_h

#include "mitkCommon.h"
#include "mitkFileReader.h"

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "mitkTbssImage.h"
#include "QuantificationExports.h"



namespace mitk
{
  //template<class TPixelType>
  class Quantification_EXPORT TbssImporter : public itk::Object {

  public:
   // typedef TPixelType PixelType;
    typedef itk::VectorImage<float,3> DataImageType; // type of the 3d vector image containing the skeletonized images
    typedef itk::VectorImage<int,3> VectorImageType; // Datatype of the tbss gradient images
    typedef itk::Image<float, 4> FloatImage4DType;
    typedef itk::ImageFileReader<FloatImage4DType> FileReaderType4D;
    typedef itk::ImageFileReader<VectorImageType> VectorReaderType;

    typedef itk::Image<float, 3> FloatImage3DType;
    typedef itk::ImageFileReader<FloatImage3DType> FileReaderType3D;



    mitkClassMacro( TbssImporter, Object )
    itkNewMacro(Self)




    mitk::TbssImage::Pointer Import();

    void SetGroupInfo(std::vector< std::pair<std::string, int> > groups)
    {
      m_Groups = groups;
    }

    std::vector< std::pair<std::string, int> > GetGroupInfo()
    {
      return m_Groups;
    }

    void SetMeasurementInfo(std::string s)
    {
      m_MeasurementInfo = s;
    }

    std::string GetMeasurementInfo()
    {
      return m_MeasurementInfo;
    }


    void SetImportVolume(mitk::Image::Pointer inputVolume)
    {
      m_InputVolume = inputVolume;
    }

  protected:

    TbssImporter(){}

    virtual ~TbssImporter(){}

    DataImageType::Pointer m_Data;
    std::vector< std::pair<std::string, int> > m_Groups;

    std::string m_MeasurementInfo;


    mitk::Image::Pointer m_InputVolume;




  };

}

#endif // __mitkTbssImporter_h
