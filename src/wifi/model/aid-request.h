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

#ifndef AIDREQUEST_H
#define AIDREQUEST_H

#include <stdint.h>
#include "ns3/buffer.h"
#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/wifi-information-element.h"
#include "ns3/vector.h"
#include "ns3/mac48-address.h"


namespace ns3 {

/**
 * \ingroup wifi
 *
 * The IEEE 802.11 AIDREQUEST Element
 *
 * \see attribute_AIDREQUEST
 */

class AidRequest : public WifiInformationElement
{
public:
	enum ServiceCharacteristic
	  {
	    SENSOR = 1,
	    OFFLOAD = 2,
	    CRITICAL_SERVICE = 4
	  };

  AidRequest ();
  ~AidRequest ();



  WifiInformationElementId ElementId () const;
  uint8_t GetInformationFieldSize () const;
  void SerializeInformationField (Buffer::Iterator start) const;
  uint8_t DeserializeInformationField (Buffer::Iterator start, uint8_t length);
    
  Buffer::Iterator Serialize (Buffer::Iterator start) const;
  /**
   * Return the serialized size of this HT capability
   * information element.
   *
   * \return the serialized size of this HT capability information element
   */
  uint16_t GetSerializedSize () const;

  void SetAidRequestSupported (uint8_t value);
  void SetListenInterval (uint32_t interval) ;
  void SetPeerStaAddress (Mac48Address addr);
  void SetServiceCharacteristic (ServiceCharacteristic sc);
  void SetGroupAddress (Mac48Address addr);
  void SetTimModeSwitch (uint8_t value);
  void SetNonTimModeSwitch (uint8_t value);

  uint32_t GetListenInterval (void) const;
  Mac48Address GetPeerStaAddress (void) const;
  ServiceCharacteristic GetServiceCharacteristic (void) const;
  Mac48Address GetGroupAddress (void) const;
  uint8_t GetGroupAddressPresent (void) const;
  uint8_t GetTimModeSwitch (void) const;
  uint8_t GetNonTimModeSwitch (void) const;
private:
  uint8_t m_aidRequestSupported;

  uint8_t m_AidRequestMode;
  uint16_t m_AidRequestInterval;
  Mac48Address m_PeerStaAddress; //Mac48Address //6 octets
  enum ServiceCharacteristic m_ServiceCharacteristic;
  Mac48Address m_GroupAddress;//Mac48Address //6 octets
  
  uint8_t m_nonTimModeSwitch;
  uint8_t m_timModeSwitch;
};

std::ostream &operator << (std::ostream &os, const AidRequest &pageS);
std::istream &operator >> (std::istream &is, AidRequest &pageS);
ATTRIBUTE_HELPER_HEADER (AidRequest);
} //namespace ns3

#endif /* AIDREQUEST_H */
