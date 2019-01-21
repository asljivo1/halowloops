/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tim.h"
#include "ns3/assert.h"
#include "ns3/log.h" //for test
#include "ns3/attribute.h"
#include "aid-response.h"


namespace ns3 {
    
NS_LOG_COMPONENT_DEFINE ("AidResponse");
    
AidResponse::AidResponse ()
{

}

AidResponse::~AidResponse ()
{
}

WifiInformationElementId
AidResponse::ElementId () const
{
  return IE_AID_RESPONSE;
}

uint8_t
AidResponse::GetInformationFieldSize () const
{
	  //NS_ASSERT (m_length < 5);
  return 0;
}

void
AidResponse::SerializeInformationField (Buffer::Iterator start) const
{
 /*start.WriteU8 (m_PagePeriod);
 start.WriteU8 ((uint8_t) m_AidResponseControl); //0-7
 start.WriteU8 ((uint8_t) (m_AidResponseControl >> 8)); //8-15
 start.WriteU8 ((uint8_t) (m_AidResponseControl >> 16)); // 16-23
 //start.WriteU32 (m_PageBitmap);
 //start.Write (m_PageBitmapP, m_length);
 NS_ASSERT (m_length < 5);
 for (uint8_t i=0; i < m_length; i++ )
   {
      start.Write (&m_PageBitmaparry[i], m_length);
   }*/
 }

uint8_t
AidResponse::DeserializeInformationField (Buffer::Iterator start, uint8_t length)
{
  /*uint8_t  m_AidResponseControl_l;
  uint8_t  m_AidResponseControl_m;
  uint8_t  m_AidResponseControl_h;
  
 // NS_LOG_UNCOND ("DeserializeInformationField length = " << length);
  
  m_PagePeriod = start.ReadU8 ();
  m_AidResponseControl_l = start.ReadU8 ();
  m_AidResponseControl_m = start.ReadU8 ();
  m_AidResponseControl_h = start.ReadU8 ();
  start.Read (m_PageBitmaparry, length - 4);
  m_PageBitmap = m_PageBitmaparry;
  
  m_AidResponseControl = (uint32_t)m_AidResponseControl_l | (uint32_t)m_AidResponseControl_m << 8 | (uint32_t) m_AidResponseControl_h << 16;

   this->SetPageindex (m_AidResponseControl & 0x00000003);
   m_AidResponseLen = (m_AidResponseControl >> 2) & 0x0000001f;
   m_AidResponseCount = (m_AidResponseControl >> 7) & 0x0000001f;
   m_BlockOffset = (m_AidResponseControl >> 12) & 0x0000001f;
   m_TIMOffset = (m_AidResponseControl >> 17) & 0x0000000f;
   m_length = length - 4;
   NS_ASSERT (m_length < 5);
  return length;*/
	return 0;
}

   ATTRIBUTE_HELPER_CPP (AidResponse);

std::ostream &
operator << (std::ostream &os, const AidResponse &rpsv)
  {
        os <<  "|" ;
        return os;
  }

std::istream &
operator >> (std::istream &is, AidResponse &rpsv)
  {
        //is >> rpsv.m_length;
        return is;
  }
} //namespace ns3





