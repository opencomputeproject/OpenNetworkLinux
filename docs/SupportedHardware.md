Hardware Support
================
Because of the HTML formatting, this page may be best viewed from
<http://opennetlinux.org/hcl>


Quanta
------
<table class="table table-striped table-hover">
<thead>
<tr class="info"> 
     <th> Device                  <th> Ports            <th> CPU                 <th> Forwarding             		<th> ONL Certified         <th> In Lab <th> OF-DPA <th> OpenNSL <th> SAI </tr>
</thead>
<tr> <td> QuantaMesh T1048-LB9    <td> 48x1G  + 4x10G   <td> FreeScale P2020       <td> Broadcom BCM56534 (Firebolt3)    	<td> Yes  <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> QuantaMesh T3048-LY2    <td> 48x10G + 4x40G   <td> FreeScale P2020       <td> Broadcom BCM56846 (Trident+)     	<td> Yes   <td> Yes  <td> No <td> No <td> No </tr> 
<tr> <td> QuantaMesh T3048-LY8    <td> 48x10G + 6x40G   <td> Intel Rangeley C2758 x86 <td> Broadcom BCM56854 (Trident2)            <td> Yes  <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> QuantaMesh T5032-LY6    <td> 32x40G  <td> Intel Rangeley C2758 x86 <td> Broadcom BCM56850 (Trident2)            <td> Yes <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> QuantaMesh T3048-LY9    <td> 48x10GT + 6x40G  <td> Intel Rangeley C2758 x86 <td> Broadcom BCM56850 (Trident2)            <td> Yes <td> Yes <td> No <td> No <td> No </tr>
</table>


Accton/Edge-Core
------
<table class="table table-striped table-hover">
<thead>
<tr class="info">
     <th> Device                  <th> Ports            <th> CPU                 <th> Forwarding             		<th> ONL Certified        <th> In Lab <th> OF-DPA <th> OpenNSL <th> SAI </tr> 
</thead>
<tr> <td> Accton AS4600-54T       <td> 48x1G  + 4x10G   <td> FreeScale P2020       <td> Broadcom BCM56540 (Apollo2)       <td> Yes  <td> Yes <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Accton AS4610-54P       <td> 48x1G + 4x10G + 2x20G <td>  Dual-core ARM Cortex A9 1GHz <td> Broadcom BCM56340 (Helix4) <td> Yes <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> Accton AS5610-52X       <td> 48x10G  + 4x40G   <td> FreeScale P2020       <td> Broadcom BCM56846 (Trident+)      <td> Yes <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> Accton AS5710-54X       <td> 48x10G + 6x40G   <td> FreeScale P2041       <td> Broadcom BCM56854 (Trident2)      <td> Yes  <td> Yes <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Accton AS6700-32X       <td> 32x40G           <td> FreeScale P2041       <td> Broadcom BCM56850 (Trident2)      <td> Yes <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> Accton AS5512-54X       <td> 48x10G + 6x40G   <td> Intel Rangeley C2538 x86 <td> MediaTek/Nephos MT3258      <td> Yes <td> Yes <td> No <td>  No <td> No </tr>
<tr> <td> Accton AS5712-54X       <td> 48x10G + 6x40G   <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56854 (Trident2)      <td> Yes <td> Yes <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Accton AS6712-32X       <td> 32x40G           <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56850 (Trident2)      <td> Yes <td> Yes <td> Yes*** <td>  Yes*** <td> No </tr>
<tr> <td> Accton AS5812-54T       <td> 48x10G + 6x40G   <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56864 (Trident2+)      <td> Yes <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> Accton AS5812-54X       <td> 48x10G + 6x40G   <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56864 (Trident2+)      <td> Yes <td> Yes <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Accton AS6812-32X       <td> 32x40G           <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56864 (Trident2+)      <td> Yes <td> Yes <td> Yes***  <td> Yes*** <td> No </tr>
<tr> <td> Accton AS7712-32X       <td> 32x100G          <td> Intel Rangeley C2538 x86 <td> Broadcom BCM56960 (Tomahawk)       <td> Yes <td> Yes <td> Yes***  <td>  Yes*** <td> No </tr>
<tr> <td> Accton AS7716-32X       <td> 32x100G          <td> Intel Xeon D-1518 x86 <td> Broadcom BCM56960 (Tomahawk)       <td> Yes <td> Yes <td> Yes*** <td>  Yes*** <td> No </tr>
<tr> <td> Accton Wedge-16X        <td> 16x40G           <td> Intel Rangeley C2550 x86 <td> Broadcom BCM56864 (Trident2+)      <td> Work In Progress** <td> Yes <td> No <td> Yes <td> No </tr>
<tr> <td> Accton (FB) Wedge 100   <td> 32x100G          <td> Intel Bay Trail E3845 x86 <td> Broadcom BCM56960 (Tomahawk)      <td> Work In Progress** <td> Yes <td> No <td> Yes <td> No </tr>
</table>

DNI/Agema
---
<table class="table table-striped table-hover">
<thead>
<tr class="info">
     <th> Device                  <th> Ports            <th> CPU                 <th> Forwarding             <th> ONL Certified        <th> In Lab <th> OF-DPA <th> OpenNSL <th> SAI </tr>
</thead>
<tr> <td> AG-7448CU               <td> 48x10G  + 4x40G  <td> FreeScale P2020       <td> Broadcom BCM56845 (Trident)     <td> Yes   <td> Yes <td> No <td> No <td> No </tr>
</table>

Dell
---
<table class="table table-striped table-hover">
<thead>
<tr class="info">
     <th> Device                  <th> Ports            <th> CPU                 <th> Forwarding             <th> ONL Certified         <th> In Lab <th> OF-DPA <th> OpenNSL <th> SAI </tr>
</thead>
<tr> <td> S4810-ON            <td> 48x10G  + 4x40G  <td> FreeScale P2020        <td> Broadcom BCM56845 (Trident)     <td> Yes   <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> S4048-ON            <td> 48x10G  + 6x40G  <td> Intel Atom C2338       <td> Broadcom BCM56854 (Trident2)     <td> Yes  <td> Yes <td> No <td> No <td> No </tr> 
<tr> <td> S6000-ON            <td> 32x40G           <td> Intel Atom S1220       <td> Broadcom BCM56850 (Trident2)     <td> Yes  <td> Yes <td> No <td> No <td> No </tr>
<tr> <td> Z9100-ON            <td> 32x100G           <td> Intel Atom C2538       <td> Broadcom BCM56960 (Tomahawk)     <td> Yes  <td> Yes <td> No <td> No <td> No </tr>
</table>

Interface Masters Technologies, Inc.
---
<table class="table table-striped table-hover">
<thead>
<tr class="info">
     <th> Device          <th> Ports       <th> CPU        <th> Forwarding       <th> ONL Certified     <th> In Lab <th> OF-DPA <th> OpenNSL <th> SAI </tr>
</thead>
<tr> <td> Niagara 2948X12XLm   <td> 48x10G  + 12x40G  <td> Intel/AMD x86    <td> Broadcom BCM56850 (Trident2)   <td> Work In Progress** <td> No <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Niagara 2960X6XLm    <td> 60x10G  + 6x40G   <td> Intel/AMD x86    <td> Broadcom BCM56850 (Trident2)   <td> Work In Progress** <td> No <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Niagara 2972Xm       <td> 72x10G            <td> Intel/AMD x86    <td> Broadcom BCM56850 (Trident2)   <td> Work In Progress** <td> Yes <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Niagara 2932XL       <td> 32x40G            <td> Intel/AMD x86    <td> Broadcom BCM56850 (Trident2)   <td> Work In Progress** <td> No <td> Yes*** <td> Yes*** <td> No </tr>
<tr> <td> Niagara 2948X6XL     <td> 48x10G  + 6x40G   <td> Intel/AMD x86    <td> Broadcom BCM56850 (Trident2)   <td> Work In Progress** <td> No <td> Yes*** <td> Yes <td> No </tr>
</table>

Mellanox
---
<table class="table table-striped table-hover">
<thead>
<tr class="info">
     <th> Device          <th> Ports       <th> CPU        <th> Forwarding       <th> ONL Certified     <th> In Lab <th> SAI </tr>
</thead>
<tr> <td> SN2100 <td> 16x100G <td> Intel Rangeley C2558 <td> Mellanox Spectrum <td> Yes <td> Yes <td> Yes </tr>
<tr> <td> SN2100B <td> 16x40G <td> Intel Rangeley C2558 <td> Mellanox Spectrum <td> Yes <td> No <td> Yes </tr>
<tr> <td> SN2410 <td> 48x25G + 8x100G <td> Intel Ivybridge 1047UE <td> Mellanox Spectrum <td> Yes <td> Yes <td> Yes </tr>
<tr> <td> SN2410B <td> 48x10G + 8x100G <td> Intel Ivybridge 1047UE <td> Mellanox Spectrum <td> Yes <td> No <td> Yes </tr>
<tr> <td> SN2700 <td> 32x100G <td> Intel Ivybridge 1047UE <td> Mellanox Spectrum <td> Yes <td> Yes <td> Yes </tr>
<tr> <td> SN2700B <td> 32x40G <td> Intel Ivybridge 1047UE <td> Mellanox Spectrum <td> Yes <td> No <td> Yes </tr>
</table>

Notes:
---

ONL Certified means that the system runs ONIE, is able to install a generic version of ONL and has the ONL Platform drivers necessary to manage the system.

\* Systems no longer in the lab cannot be certified post removal

\** Developing ONL Platform Drivers

\*** Vendor provided
