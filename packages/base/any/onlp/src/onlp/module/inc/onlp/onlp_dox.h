/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#ifndef __ONLP_DOX_H__
#define __ONLP_DOX_H__

/**

@mainpage This is the main page.

This is text.
@section s1 Section1
Foobar
@section s2 Section2
Foobar2

@defgroup onlp-main ONLP
@{

  @defgroup onlp-appication ONLP Application Interfaces
  @{

    @defgroup system-interfaces General System Interfaces
    These are general system interfaces.
    @{
      @defgroup platform Platform Initialization and Management
      @defgroup stdattrs Standard Attribute Structures
    @}
    @defgroup oid-interfaces General OID Interfaces
    These interfaces are available on all OIDs.
    @{
      @defgroup attributes Attribute Interface
      @defgroup assets Asset Interface
      @defgroup onie   ONIE Interface
    @}

    @defgroup oid-types OID Types
    Software interfaces by OID Type.
    @{
      @defgroup oid-chassis Chassis
      @defgroup oid-module  Modules
      @defgroup oid-thermal Thermals
      @defgroup oid-fan Fans
      @defgroup oid-psu PSUs
      @defgroup oid-sfp SFPs
      @defgroup oid-led LEDs
      @defgroup oid-generic Generics
    @}
  @}

  @defgroup onlp-platform Platform Implementation Interfaces
  These document the requires for implementing the platform interfaces.
  @{
     @defgroup chassisi Chassis
     @defgroup modulei  Modules
     @defgroup thermali Thermals
     @defgroup fani     Fans
     @defgroup psui     PSUs
     @defgroup sfpi     SFPs
  @}

  @defgroup module-interfaces Module Documentation
  @{
    @defgroup onlp-config Compile Time Configuration
    @defgroup onlp-porting Porting Macros
  @}
@}

*/

#endif /* __ONLP_DOX_H__ */
