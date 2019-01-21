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

#ifndef AIDRESPONSE_H
#define AIDRESPONSE_H

#include <stdint.h>
#include "ns3/buffer.h"
#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/wifi-information-element.h"
#include "ns3/vector.h"


namespace ns3 {

/**
 * \ingroup wifi
 *
 * The IEEE 802.11 AidResponse Element
 *
 * \see attribute_AidResponse
 */
class AidResponse : public WifiInformationElement
{
public:
  AidResponse ();
  ~AidResponse ();


  WifiInformationElementId ElementId () const;
  uint8_t GetInformationFieldSize () const;
  void SerializeInformationField (Buffer::Iterator start) const;
  uint8_t DeserializeInformationField (Buffer::Iterator start, uint8_t length);
    
  //uint8_t m_length; //!< length of m_PageBitmap field*/

private:
  uint8_t m_Aid;
  uint32_t m_AidSwitchCount;
  uint8_t m_AidResponseInterval;
  
};

std::ostream &operator << (std::ostream &os, const AidResponse &pageS);
std::istream &operator >> (std::istream &is, AidResponse &pageS);
ATTRIBUTE_HELPER_HEADER (AidResponse);
} //namespace ns3

#endif /* AIDRESPONSE_H */
