/*==========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkExtensionVelocitiesImageFilter.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef _itkExtensionVelocitiesImageFilter_txx
#define _itkExtensionVelocitiesImageFilter_txx

#include "itkImageRegionIterator.h"
#include "itkIndex.h"


namespace itk
{

/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::ExtensionVelocitiesImageFilter()
{

  m_Locator = LocatorType::New();
  m_Marcher = FastMarchingImageFilterType::New();

  this->ProcessObject::SetNumberOfRequiredInputs(VAuxDimension + 1);
  this->ProcessObject::SetNumberOfRequiredOutputs(VAuxDimension + 1);

  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    AuxImagePointer ptr;
    ptr = AuxImageType::New();
    this->ProcessObject::SetNthOutput( k+1, ptr.GetPointer() );
    }

}

/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::SetInputVelocityImage(
AuxImageType * ptr,
unsigned int idx )
{
  if( !ptr || idx >= VAuxDimension )
    {
    return;
    }

  this->ProcessObject::SetNthInput( idx+1, ptr );

}


/**
 * 
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension>
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::AuxImagePointer
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::GetInputVelocityImage( unsigned int idx )
{

  if ( idx >= VAuxDimension || this->GetNumberOfInputs() < idx )
    {
    return NULL;
    }

  return static_cast<AuxImageType*>(
    this->ProcessObject::GetInput(idx+1).GetPointer() );

}


/**
 * 
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension>
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::AuxImagePointer
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::GetOutputVelocityImage( unsigned int idx )
{

  if ( idx >= VAuxDimension || this->GetNumberOfOutputs() < idx )
    {
    return NULL;
    }

  return static_cast<AuxImageType*>(
    this->ProcessObject::GetOutput(idx+1).GetPointer() );

}


/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::GenerateInputRequestedRegion()
{
  // call the superclass implemenation of this method
  this->Superclass::GenerateInputRequestedRegion();

  // this filter requires all of the input images to be
  // in the buffer
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    this->GetInputVelocityImage(k)->
      SetRequestedRegionToLargestPossibleRegion();
    }

}


/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::EnlargeOutputRequestedRegion( DataObject * output )
{
  // call the superclass implemenation of this method
  this->Superclass::EnlargeOutputRequestedRegion( output );

  // set the requested region for all output to be the
  // same as the primary input
  LevelSetPointer primaryOutput = this->GetOutput(0);
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    this->GetOutputVelocityImage(k)->SetRequestedRegion(
      primaryOutput->GetRequestedRegion() );
    }

}


/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::AllocateOutput()
{
  this->Superclass::AllocateOutput();

  // allocate memory for the output images
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    AuxImagePointer output = this->GetOutputVelocityImage(k);
    output->SetBufferedRegion( output->GetRequestedRegion() );
    output->Allocate();
    }

  // set the marcher output size
  LevelSetPointer outputPtr = this->GetOutput();
  m_Marcher->SetOutputSize( 
    outputPtr->GetRequestedRegion().GetSize() );

}

/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::GenerateDataFull()
{

  LevelSetPointer inputPtr = this->GetInput();
  LevelSetPointer outputPtr = this->GetOutput();
  LevelSetPointer tempLevelSet = m_Marcher->GetOutput();

  double levelSetValue = this->GetLevelSetValue();

  // define iterators
  typedef 
    ImageRegionIterator<LevelSetType::LevelSetImageType> IteratorType;

  IteratorType inputIt( inputPtr,
    inputPtr->GetBufferedRegion() );
  IteratorType outputIt( outputPtr,
    outputPtr->GetBufferedRegion() );

  IteratorType tempIt;

  typedef
    ImageRegionIterator<AuxImageType> AuxIteratorType;

  AuxIteratorType auxTempIt[VAuxDimension];
  AuxIteratorType auxOutputIt[VAuxDimension];

  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    AuxImagePointer ptr = this->GetOutputVelocityImage(k);
    auxOutputIt[k] = AuxIteratorType( ptr,
      ptr->GetBufferedRegion() );
    }
 
  this->UpdateProgress( 0.0 );

  // locate the level set
  m_Locator->SetInputLevelSet( inputPtr );
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    m_Locator->SetAuxImage( this->GetInputVelocityImage(k), k );
    }
  m_Locator->SetLevelSetValue( levelSetValue );
  m_Locator->Locate();


  this->UpdateProgress( 0.33 );

  // march outward
  m_Marcher->SetTrialPoints( m_Locator->GetOutsidePoints() );
  m_Marcher->SetAuxiliaryTrialValues( m_Locator->GetAuxOutsideValues() );
  m_Marcher->Update();

  tempIt = IteratorType( tempLevelSet,
    tempLevelSet->GetBufferedRegion() );

  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    AuxImagePointer ptr;
    ptr = m_Marcher->GetAuxiliaryImage(k);
    auxTempIt[k] = AuxIteratorType( ptr,
                               ptr->GetBufferedRegion() );
    }

  double value;

  inputIt.GoToBegin();
  outputIt.GoToBegin();
  tempIt.GoToBegin();
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    auxOutputIt[k].GoToBegin();
    auxTempIt[k].GoToBegin();
    }

  while( !inputIt.IsAtEnd() )
    {
    value = (double) inputIt.Get();
    if( value - levelSetValue > 0 )
      {
      outputIt.Set( tempIt.Get() );

      for( unsigned int k = 0; k < VAuxDimension; k++ )
        {
        auxOutputIt[k].Set( auxTempIt[k].Get() );
        }

     }

    ++inputIt;
    ++outputIt;
    ++tempIt;
    for( unsigned int k = 0; k < VAuxDimension; k++ )
      {
      ++(auxTempIt[k]);
      ++(auxOutputIt[k]);
      }
    }

  this->UpdateProgress( 0.66 );

  // march inward
  m_Marcher->SetTrialPoints( m_Locator->GetInsidePoints() );
  m_Marcher->SetAuxiliaryTrialValues( m_Locator->GetAuxInsideValues() );
  m_Marcher->Update();

  inputIt.GoToBegin();
  outputIt.GoToEnd();
  tempIt.GoToBegin();
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    auxOutputIt[k].GoToBegin();
    auxTempIt[k].GoToBegin();
    }

  while( !inputIt.IsAtEnd() )
    {
    value = (double) inputIt.Get();
    if( value - levelSetValue <= 0 )
      {
      value = (double) tempIt.Get();
      outputIt.Set( -1.0 * value );

      for( unsigned int k = 0; k < VAuxDimension; k++ )
        {
        auxOutputIt[k].Set( auxTempIt[k].Get() );
        }
      }

    ++inputIt;
    ++outputIt;
    ++tempIt;
    for( unsigned int k = 0; k < VAuxDimension; k++ )
      {
      ++(auxTempIt[k]);
      ++(auxOutputIt[k]);
      }
    }

}
  

/**
 *
 */
template <class TLevelSet, class TAuxValue, unsigned int VAuxDimension >
void
ExtensionVelocitiesImageFilter<TLevelSet,TAuxValue,VAuxDimension>
::GenerateDataNarrowBand()
{

  LevelSetPointer inputPtr = this->GetInput();
  LevelSetPointer outputPtr = this->GetOutput();
  LevelSetPointer tempLevelSet = m_Marcher->GetOutput();

  double levelSetValue = this->GetLevelSetValue();
  double outputBandwidth = this->GetOutputNarrowBandwidth();
  double inputBandwidth = this->GetInputNarrowBandwidth();

  // define iterators
  typedef 
    ImageRegionIterator<LevelSetType::LevelSetImageType> IteratorType;

  IteratorType inputIt( inputPtr,
    inputPtr->GetBufferedRegion() );

  IteratorType outputIt( outputPtr,
    outputPtr->GetBufferedRegion() );

  PixelType posInfinity;
  PixelType negInfinity;

  posInfinity = NumericTraits<PixelType>::max();
  negInfinity = NumericTraits<PixelType>::NonpositiveMin();

  // set all internal pixels to minus infinity and 
  // all external pixels to positive infinity
  double value;

  inputIt.GoToBegin();
  outputIt.GoToBegin();

  while( !inputIt.IsAtEnd() )
    {
    value = (double) inputIt.Get();
    if( value - levelSetValue <= 0 )
      {
      outputIt.Set( negInfinity );
      }
    else
      {
      outputIt.Set( posInfinity );
      }

    ++inputIt;
    ++outputIt;
    }

  // set all auxiliary images to zero
  TAuxValue zeroPixel;
  zeroPixel = 0.0;

  typedef
    ImageRegionIterator<AuxImageType> AuxIteratorType;

  AuxIteratorType auxOutputIt[VAuxDimension];

  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    AuxImagePointer ptr = this->GetOutputVelocityImage(k);
    auxOutputIt[k] = AuxIteratorType( ptr,
      ptr->GetBufferedRegion() );
    auxOutputIt[k].GoToBegin();
    }
  while( !auxOutputIt[0].IsAtEnd() )
    {
    for( unsigned int k = 0; k < VAuxDimension; k++ )
      {
      auxOutputIt[k].Set(zeroPixel);
      ++(auxOutputIt[k]);

      }
    }

  AuxImagePointer tempAuxImage[VAuxDimension];
  AuxImagePointer outputAuxImage[VAuxDimension];
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    tempAuxImage[k] = m_Marcher->GetAuxiliaryImage(k);
    outputAuxImage[k] = this->GetOutputVelocityImage(k);
    }

  // create a new output narrowband container
  NodeContainerPointer outputNB = NodeContainer::New();
  this->SetOutputNarrowBand( outputNB );

  this->UpdateProgress( 0.0 );

  // locate the level set
  m_Locator->SetInputLevelSet( inputPtr );
  for( unsigned int k = 0; k < VAuxDimension; k++ )
    {
    m_Locator->SetAuxImage( this->GetInputVelocityImage(k), k );
    }
  m_Locator->SetLevelSetValue( levelSetValue );

  if( this->GetNarrowBanding() && this->GetInputNarrowBand() )
    {
    m_Locator->NarrowBandingOn();
    m_Locator->SetNarrowBandwidth( inputBandwidth );
    m_Locator->SetInputNarrowBand( this->GetInputNarrowBand() );
    }
  else
    { 
    m_Locator->NarrowBandingOff();
    }

  m_Locator->Locate();

  this->UpdateProgress( 0.33 );


  // march outward
  double stoppingValue = ( outputBandwidth / 2.0 ) + 2.0;
  m_Marcher->SetStoppingValue( stoppingValue );
  m_Marcher->CollectPointsOn();
  m_Marcher->SetTrialPoints( m_Locator->GetOutsidePoints() );
  m_Marcher->SetAuxiliaryTrialValues( m_Locator->GetAuxOutsideValues() );
  m_Marcher->Update();

  NodeContainerPointer procPoints = m_Marcher->GetProcessedPoints();
  
  typename NodeContainer::ConstIterator pointsIt;
  typename NodeContainer::ConstIterator pointsEnd;
  
  pointsIt = procPoints->Begin();
  pointsEnd = procPoints->End();

  NodeType node;
  PixelType inPixel;

  for( ; pointsIt != pointsEnd; ++pointsIt )
    {
    node = pointsIt.Value();
    inPixel = inputPtr->GetPixel( node.GetIndex() );
    
    value = (double) inPixel;
    if( value - levelSetValue > 0 )
      {
      inPixel = tempLevelSet->GetPixel( node.GetIndex() );
      outputPtr->SetPixel( node.GetIndex(), inPixel );
      outputNB->InsertElement( outputNB->Size(), node );

      for( unsigned int k = 0; k < VAuxDimension; k++ )
        { 
        outputAuxImage[k]->SetPixel( node.GetIndex(), 
          tempAuxImage[k]->GetPixel( node.GetIndex() ) );
        }

      }

    }

  this->UpdateProgress( 0.66 );

  // march inward
  m_Marcher->SetTrialPoints( m_Locator->GetInsidePoints() );
  m_Marcher->SetAuxiliaryTrialValues( m_Locator->GetAuxInsideValues() );
  m_Marcher->Update();

  procPoints = m_Marcher->GetProcessedPoints();
  pointsIt = procPoints->Begin();
  pointsEnd = procPoints->End();

  for( ; pointsIt != pointsEnd; ++pointsIt )
    {
    node = pointsIt.Value();
    inPixel = inputPtr->GetPixel( node.GetIndex() );
    
    value = (double) inPixel;
    if( value - levelSetValue <= 0 )
      {
      inPixel = tempLevelSet->GetPixel( node.GetIndex() );
      value = (double) inPixel;
      inPixel = -1.0 * value;
      outputPtr->SetPixel( node.GetIndex(), inPixel );
      node.SetValue( node.GetValue() * -1.0 );
      outputNB->InsertElement( outputNB->Size(), node );

      for( unsigned int k = 0; k < VAuxDimension; k++ )
        { 
        outputAuxImage[k]->SetPixel( node.GetIndex(), 
          tempAuxImage[k]->GetPixel( node.GetIndex() ) );
        }

      }

    }

}


} //namespace itk

#endif
