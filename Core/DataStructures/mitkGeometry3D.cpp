/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkGeometry3D.h"
#include "mitkOperation.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include "mitkStatusBar.h"
#include <float.h>

#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

// Standard Constructor for the new makro. sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() 
  : m_ParametricBoundingBox(NULL),
    m_ImageGeometry(false), m_Valid(true), m_FrameOfReferenceID(0)
{
  FillVector3D(m_FloatSpacing, 1,1,1);
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  Initialize();
}

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{

}

static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3])
{
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = transform->GetMatrix().GetVnlMatrix();

  spacing[0]=vnlmatrix.get_column(0).magnitude();
  spacing[1]=vnlmatrix.get_column(1).magnitude();
  spacing[2]=vnlmatrix.get_column(2).magnitude();
  mitk::itk2vtk(spacing, floatSpacing);
}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize()
{
  float b[6] = {0,1,0,1,0,1};
  SetFloatBounds(b);

  m_IndexToObjectTransform = TransformType::New();
  m_ObjectToNodeTransform = TransformType::New();

  if(m_IndexToWorldTransform.IsNull())
    m_IndexToWorldTransform = TransformType::New();
  else
    m_IndexToWorldTransform->SetIdentity();
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);

  m_VtkMatrix->Identity();
 
  m_ParametricTransform = m_IndexToWorldTransform;

  m_TimeBoundsInMS[0]=-ScalarTypeNumericTraits::max(); m_TimeBoundsInMS[1]=ScalarTypeNumericTraits::max();

  m_FrameOfReferenceID = 0;
}

void mitk::Geometry3D::TransferItkToVtkTransform()
{
 // copy m_IndexToWorldTransform into m_VtkIndexToWorldTransform 
  int i,j;
  for(i=0;i<3;++i)
    for(j=0;j<3;++j)
      m_VtkMatrix->SetElement(i, j, m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get(i, j));
  for(i=0;i<3;++i)
    m_VtkMatrix->SetElement(i, 3, m_IndexToWorldTransform->GetOffset()[i]);
  m_VtkIndexToWorldTransform->Modified();
}

void mitk::Geometry3D::TransferVtkToItkTransform()
{
  itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType& vlnMatrix = const_cast<itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType&>(m_IndexToWorldTransform->GetMatrix().GetVnlMatrix());  

  for ( int i=0; i < 3; ++i)
    for( int j=0; j < 3; ++j )
      vlnMatrix.put( i, j, m_VtkMatrix->GetElement( i, j ) );      

  itk::AffineTransform<mitk::ScalarType>::OffsetType offset;
  offset[0] = m_VtkMatrix->GetElement( 0, 3 );
  offset[1] = m_VtkMatrix->GetElement( 1, 3 );
  offset[2] = m_VtkMatrix->GetElement( 2, 3 );
  m_IndexToWorldTransform->SetOffset( offset );
  m_IndexToWorldTransform->Modified();
  m_ParametricTransform = m_IndexToWorldTransform;
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
}

void mitk::Geometry3D::SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix)
{
  m_VtkMatrix->DeepCopy(vtkmatrix);
  TransferVtkToItkTransform();
}

void mitk::Geometry3D::SetTimeBoundsInMS(const TimeBounds& timebounds)
{
  if(m_TimeBoundsInMS != timebounds)
  {
    m_TimeBoundsInMS = timebounds;
    Modified();
  }
}
  
void mitk::Geometry3D::SetFloatBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const float *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBoundsArray(b, m_BoundingBox);
}
  
void mitk::Geometry3D::SetFloatBounds(const double bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const double *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBoundsArray(b, m_BoundingBox);
}

void mitk::Geometry3D::SetParametricBounds(const BoundingBox::BoundsArrayType& bounds)
{
	//std::cout << " BOUNDS: " << bounds << std::endl;
  SetBoundsArray(bounds, m_ParametricBoundingBox);
}

void mitk::Geometry3D::WorldToIndex(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const
{
  BackTransform(pt_mm, pt_units);
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::IndexToWorld(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  pt_mm = m_ParametricTransform->TransformPoint(pt_units);
}

//##ModelId=3E3B986602CF
void mitk::Geometry3D::WorldToIndex(const mitk::Point3D &atPt3d_mm, const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  BackTransform(atPt3d_mm, vec_mm, vec_units);
}

//##ModelId=3E3B987503A3
void mitk::Geometry3D::IndexToWorld(const mitk::Point3D &atPt3d_units, const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  vec_mm = m_ParametricTransform->TransformVector(vec_units);
}

void mitk::Geometry3D::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  if(m_IndexToWorldTransform.GetPointer() != transform)
  {
    Superclass::SetIndexToWorldTransform(transform);
    m_ParametricTransform = m_IndexToWorldTransform;
    CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
    TransferItkToVtkTransform();
    Modified();
  }
}

mitk::AffineGeometryFrame3D::Pointer mitk::Geometry3D::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::Geometry3D::InitializeGeometry(Geometry3D * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);

  newGeometry->SetTimeBoundsInMS(m_TimeBoundsInMS);  

  //newGeometry->GetVtkTransform()->SetMatrix(m_VtkIndexToWorldTransform->GetMatrix()); IW
	//newGeometry->TransferVtkToItkTransform(); //MH


  if(m_ParametricBoundingBox.IsNotNull())
    newGeometry->SetParametricBounds(m_ParametricBoundingBox->GetBounds());

  newGeometry->SetFrameOfReferenceID(GetFrameOfReferenceID());
}

void mitk::Geometry3D::SetExtentInMM(int direction, ScalarType extentInMM)
{
  ScalarType len = GetExtentInMM(direction);
  if(fabs(len - extentInMM)>=mitk::eps)
  {
    AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
    vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();
    if(len>extentInMM)
      vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)/len*extentInMM);
    else
      vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)*extentInMM/len);
    Matrix3D matrix;
    matrix = vnlmatrix;
    m_IndexToWorldTransform->SetMatrix(matrix);
    Modified();
  }
}

mitk::BoundingBox::Pointer mitk::Geometry3D::CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform)
{
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;

  unsigned char i;
  if(transform!=NULL)
  {
    for(i=0; i<8; ++i)
      pointscontainer->InsertElement( pointid++, transform->BackTransformPoint( GetCornerPoint(i) ));
  }
  else
  {
    for(i=0; i<8; ++i)
      pointscontainer->InsertElement( pointid++, GetCornerPoint(i) );
  }

  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
}

#include <vtkTransform.h>
void mitk::Geometry3D::ExecuteOperation(Operation* operation)
{
  vtkTransform *vtktransform = vtkTransform::New();
  vtktransform->SetMatrix(m_VtkMatrix);
  switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpMOVE:
  {
    mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	  if (pointOp == NULL)
	  {
		  mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::Point3D newPos = pointOp->GetPoint();
    ScalarType data[3];
    vtktransform->GetPosition(data);
    vtktransform->PostMultiply();
    vtktransform->Translate(newPos[0], newPos[1], newPos[2]);    
    vtktransform->PreMultiply();
    break;
  }
	case OpSCALE:
  {
    mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	  if (pointOp == NULL)
	  {
		  mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::Point3D newScale = pointOp->GetPoint();
    ScalarType data[3];
    /* calculate new scale: newscale = oldscale * (oldscale + scaletoadd)/oldscale */
    data[0] = 1 + (newScale[0] / GetXAxis().GetNorm());
    data[1] = 1 + (newScale[1] / GetYAxis().GetNorm());
    data[2] = 1 + (newScale[2] / GetZAxis().GetNorm());  

    mitk::Point3D center = const_cast<mitk::BoundingBox*>(m_BoundingBox.GetPointer())->GetCenter();
    ScalarType pos[3];
    vtktransform->GetPosition(pos);
    vtktransform->PostMultiply();
    vtktransform->Translate(-pos[0], -pos[1], -pos[2]);
    vtktransform->Translate(-center[0], -center[1], -center[2]);   
    vtktransform->PreMultiply();
    vtktransform->Scale(data[0], data[1], data[2]);
    vtktransform->PostMultiply();
    vtktransform->Translate(+center[0], +center[1], +center[2]);   
    vtktransform->Translate(pos[0], pos[1], pos[2]);
    vtktransform->PreMultiply();
    break;
  }
  case OpROTATE:
  {
    mitk::RotationOperation *rotateOp = dynamic_cast<mitk::RotationOperation *>(operation);
    if (rotateOp == NULL)
    {
	    mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
	    return;
    }
    Vector3D rotationVector = rotateOp->GetVectorOfRotation();
    Point3D center = rotateOp->GetCenterOfRotation();
    ScalarType angle = rotateOp->GetAngleOfRotation();
    angle = (angle < -360) ? -360 : angle;
    angle = (angle >  360) ?  360 : angle;
    vtktransform->PostMultiply();
    vtktransform->Translate(-center[0], -center[1], -center[2]);
    vtktransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
    vtktransform->Translate(center[0], center[1], center[2]);
    vtktransform->PreMultiply();
    break;  
  }
  default:
    return;
	}
  m_VtkMatrix->DeepCopy(vtktransform->GetMatrix());
  TransferVtkToItkTransform();
}

void mitk::Geometry3D::BackTransform(const mitk::Point3D &in, mitk::Point3D& out) const
{
  ScalarType temp[3];
  unsigned int i, j;
  const TransformType::OffsetType& offset = m_IndexToWorldTransform->GetOffset();

  for (j = 0; j < 3; j++) 
  {
    temp[j] = in[j] - offset[j];
  }

#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR <= 8)
  const TransformType::MatrixType& inverse = m_IndexToWorldTransform->GetInverse();
#else
  const TransformType::MatrixType& inverse = m_IndexToWorldTransform->GetInverseMatrix();
#endif
  for (i = 0; i < 3; i++) 
  {
    out[i] = 0.0;
    for (j = 0; j < 3; j++) 
    {
      out[i] += inverse[i][j]*temp[j];
    }
  }
}

void mitk::Geometry3D::BackTransform(const mitk::Point3D &at, const mitk::Vector3D &in, mitk::Vector3D& out) const
{
#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR <= 8)
  const TransformType::MatrixType& inverse = m_IndexToWorldTransform->GetInverse();
#else
  const TransformType::MatrixType& inverse = m_IndexToWorldTransform->GetInverseMatrix();
#endif
  out = inverse * in;
}

const float* mitk::Geometry3D::GetFloatSpacing() const
{
  return m_FloatSpacing;
}

void mitk::Geometry3D::SetSpacing(const float aSpacing[3])
{
  mitk::Vector3D tmp;
  tmp[0]= aSpacing[0];
  tmp[1]= aSpacing[1];
  tmp[2]= aSpacing[2];
  SetSpacing(tmp);
}

void mitk::Geometry3D::SetSpacing(const mitk::Vector3D& aSpacing)
{
  if(mitk::Equal(m_Spacing, aSpacing) == false)
  {
    assert(aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0);

    m_Spacing = aSpacing;

    AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;

    vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();

    mitk::VnlVector col;
    col = vnlmatrix.get_column(0); col.normalize(); col*=aSpacing[0]; vnlmatrix.set_column(0, col);
    col = vnlmatrix.get_column(1); col.normalize(); col*=aSpacing[1]; vnlmatrix.set_column(1, col);
    col = vnlmatrix.get_column(2); col.normalize(); col*=aSpacing[2]; vnlmatrix.set_column(2, col);

    Matrix3D matrix;
    matrix = vnlmatrix;

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(m_IndexToWorldTransform->GetOffset());

    SetIndexToWorldTransform(transform.GetPointer());

    itk2vtk(m_Spacing, m_FloatSpacing);
  }
}

void mitk::Geometry3D::Translate(const Vector3D & vector)
{
  m_IndexToWorldTransform->SetOffset(m_IndexToWorldTransform->GetOffset()+vector);
  TransferItkToVtkTransform();
  Modified();
}

const mitk::Vector3D mitk::Geometry3D::GetXAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][0];
  v[1] = m->Element[1][0];
  v[2] = m->Element[2][0];
  return v;
}

const mitk::Vector3D mitk::Geometry3D::GetYAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][1];
  v[1] = m->Element[1][1];
  v[2] = m->Element[2][1];
  return v;
}
const mitk::Vector3D mitk::Geometry3D::GetZAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][2];
  v[1] = m->Element[1][2];
  v[2] = m->Element[2][2];
  return v;
}
