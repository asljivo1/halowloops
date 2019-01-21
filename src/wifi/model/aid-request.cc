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
#include "aid-request.h"
#include <cmath>


namespace ns3 {
    
NS_LOG_COMPONENT_DEFINE ("AidRequest");
    
AidRequest::AidRequest ()
: m_aidRequestSupported (0),
  m_AidRequestInterval (0),
  m_nonTimModeSwitch (0),
  m_timModeSwitch (0),
  m_AidRequestMode (0)
{
	//Aid Request field is empty by default and not used in Association Request frames
}

AidRequest::~AidRequest ()
{
}

WifiInformationElementId
AidRequest::ElementId () const
{
  return IE_AID_REQUEST;
}

uint8_t
AidRequest::GetGroupAddressPresent (void) const
{
	return ((m_AidRequestMode >> 5) & 0x0001);
}

uint8_t
AidRequest::GetTimModeSwitch (void) const
{
	return ((m_AidRequestMode >> 4) & 0x0001);
}

uint8_t
AidRequest::GetNonTimModeSwitch (void) const
{
	return ((m_AidRequestMode >> 3) & 0x0001);
}

uint32_t
AidRequest::GetListenInterval (void) const
{
	uint8_t unifiedScalingFactor = m_AidRequestInterval & 0x00000011;
	uint16_t unscaledInterval = m_AidRequestInterval >> 2;
	if (unifiedScalingFactor < 2)
		return unscaledInterval * pow(10, unifiedScalingFactor);
	else if (unifiedScalingFactor == 2)
		return unscaledInterval * 1000;
	else if (unifiedScalingFactor == 3)
		return unscaledInterval * 10000 ;
	else
		return 0;
}

Mac48Address
AidRequest::GetPeerStaAddress (void) const
{
	return m_PeerStaAddress;
}

AidRequest::ServiceCharacteristic
AidRequest::GetServiceCharacteristic (void) const
{
	return m_ServiceCharacteristic;
}

Mac48Address
AidRequest::GetGroupAddress (void) const
{
	return m_GroupAddress;
}

void
AidRequest::SetAidRequestSupported (uint8_t value)
{
	m_aidRequestSupported = value;
}

void
AidRequest::SetPeerStaAddress (Mac48Address addr)
{
	m_PeerStaAddress = addr;
	m_AidRequestMode |= (0x0001 << 1);
}

void
AidRequest::SetServiceCharacteristic (AidRequest::ServiceCharacteristic sc)
{
	m_ServiceCharacteristic = sc;
	m_AidRequestMode |= (0x0001 << 2);
}

void
AidRequest::SetGroupAddress (Mac48Address addr)
{
	m_GroupAddress = addr;
	m_AidRequestMode |= (0x0001 << 5);
}

void
AidRequest::SetTimModeSwitch (uint8_t value)
{
	m_AidRequestMode |= (0x0001 << 4);
}

void
AidRequest::SetNonTimModeSwitch (uint8_t value)
{
	m_AidRequestMode |= (0x0001 << 3);
}

//9.4.1.6 Listen Interval Field
//how many BIs will the TIM station skip or in how many BIs the non-TIM station has to poll the AP
void
AidRequest::SetListenInterval(uint32_t interval)
{
	NS_LOG_FUNCTION (this);
	//NS_ASSERT (GetNonTimModeSwitch () == 0 && GetTimModeSwitch () == 1 && GetGroupAddressPresent () == 0);
	NS_ASSERT (interval < 163840000);
	if (interval == 0)
	{
		m_AidRequestInterval = 0;
		m_AidRequestMode &= ~(1UL);
		return;
	}
	if (interval < 16384)
	{
		m_AidRequestInterval = (interval << 2);
	}
	else if (interval < 163840)
	{
		m_AidRequestInterval = ((interval/10 << 2) | 0x00000001);
	}
	else if (interval < 16384000)
	{
		m_AidRequestInterval = ((interval/1000 << 2) | 0x00000010);
	}
	else if (interval < 163840000)
	{
		m_AidRequestInterval = ((interval/10000 << 2) | 0x00000011);
	}
	m_AidRequestMode |= 0x0001;
}

uint8_t
AidRequest::GetInformationFieldSize () const
{
  NS_ASSERT (m_aidRequestSupported > 0);
  uint8_t length = 1 + 2 * (m_AidRequestMode & 0x0001) + 6 * ((m_AidRequestMode & 0x0002) >> 1) + ((m_AidRequestMode & 0x0004) >> 2) + 6 * ((m_AidRequestMode & 0x0020) >> 5);
  return length;
}

void
AidRequest::SerializeInformationField (Buffer::Iterator start) const
{
	if (m_aidRequestSupported > 0)
	{
		start.WriteU8 (m_AidRequestMode);
		if (m_AidRequestMode & 0x0001)
		{
			start.WriteU16(this->m_AidRequestInterval);
		}
		if (m_AidRequestMode & 0x0002)
		{
			uint8_t buffer[6];
			m_PeerStaAddress.CopyTo(buffer);
			start.Write(buffer, 6);

		}
		if (m_AidRequestMode & 0x0004)
		{
			start.WriteU8(static_cast<uint8_t> (this->m_ServiceCharacteristic));

		}
		if (m_AidRequestMode & 0x0020)
		{
			uint8_t buffer[6];
			m_GroupAddress.CopyTo(buffer);
			start.Write(buffer, 6);
		}
		/*NS_LOG_UNCOND ("AidRequest::SerializeInformationField " << ", m_AidRequestMode=" << (int)m_AidRequestMode
				<< ", m_AidRequestInterval=" << (int)m_AidRequestInterval << ", m_PeerStaAddress=" << m_PeerStaAddress << ", m_ServiceCharacteristic=" << static_cast<int> (this->m_ServiceCharacteristic)
				<< ", m_GroupAddress=" << m_GroupAddress);*/
	}
}

Buffer::Iterator
AidRequest::Serialize (Buffer::Iterator i) const
{
  if (m_aidRequestSupported < 1)
    {
      return i;
    }
  return WifiInformationElement::Serialize (i);
}

uint16_t
AidRequest::GetSerializedSize () const
{
  if (m_aidRequestSupported < 1)
    {
      return 0;
    }
  return WifiInformationElement::GetSerializedSize ();
}

uint8_t
AidRequest::DeserializeInformationField (Buffer::Iterator start, uint8_t length)
{
	//NS_LOG_UNCOND ("DeserializeInformationField length = " << length);
	m_AidRequestMode = start.ReadU8 ();
	if (m_AidRequestMode & 0x0001)
	{
		m_AidRequestInterval = start.ReadU16();
	}
	if (m_AidRequestMode & 0x0002)
	{
		uint8_t buffer[6];
		start.Read(buffer, 6);
		m_PeerStaAddress.CopyFrom(buffer);

	}
	if (m_AidRequestMode & 0x0004)
	{
		uint8_t val = start.ReadU8();
		this->SetServiceCharacteristic (static_cast<AidRequest::ServiceCharacteristic> (val));
	}
	if (m_AidRequestMode & 0x0020)
	{
		uint8_t buffer[6];
		start.Read(buffer, 6);
		m_GroupAddress.CopyFrom(buffer);
	}
	/*NS_LOG_UNCOND ("AidRequest::DeserializeInformationField length = " << (int)length << ", m_AidRequestMode=" << (int)m_AidRequestMode
			<< ", m_AidRequestInterval=" << (int)m_AidRequestInterval << ", m_PeerStaAddress=" << m_PeerStaAddress << ", m_ServiceCharacteristic=" << m_ServiceCharacteristic
			<< ", m_GroupAddress=" << m_GroupAddress);*/
	return length;
}

ATTRIBUTE_HELPER_CPP (AidRequest);

std::ostream &
operator << (std::ostream &os, const AidRequest &rpsv)
{
	os <<  "|" ;
	return os;
}

std::istream &
operator >> (std::istream &is, AidRequest &rpsv)
  {
        //is >> rpsv.m_length;
        return is;
  }
} //namespace ns3





