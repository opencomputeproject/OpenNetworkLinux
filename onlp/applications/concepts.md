# ONLP API Concepts

## Overview

The ONLP Application API manages the following types of platform objects:
* SFPs
* Fans
* PSUs
* Thermals Sensors
* LEDs
* Hot Swapped Modules
* Private/Custom features.

Platform objects in the ONLP API are represented as handles called Object IDs (OIDs). OIDs have both common and type-specific status, capabilities, and attributes.

You interact with OIDs through two types of APIs -- one common to all OIDs and one specific to the type of OID.

Attributes common to all OIDs represent status at a meta level. Is a thing present or missing? Is it operational or has it failed?

Attributes specific to a particular type of OID represent that type's functionality. You can ask a Fan its current RPM but not an LED.

There are functionality-specific APIs defined for each OID subtype.


## OID Representation

OIDs are just 32 bit integer handles. The most significant byte of the OID encodes the type (SFP, FAN, PSU, etc). The remaining 3 bytes are a unique id referencing a platform-defined object (like Fan 1, Fan 2, Fan 3).
The code that defines and manipulates OIDs can be found [here](https://github.com/opencomputeproject/OpenNetworkLinux/blob/ONLPv2/packages/base/any/onlp/src/onlp/module/inc/onlp/oids.h)


## OID Relationships

All OIDs have a parent OID. Some OIDs also have one or more child OIDs. This represents their current physical or logical relationship.

For example, a PSU typically has both a Fan and a Thermal sensor built into it. The associated Fan and Thermal sensor for a given PSU are represented as OIDs that are children of the PSU.
You can interrogate the Fan OID from the PSU using the normal Fan API, but the Fan itself cannot be accessed if the PSU is not also present.

These parent/child relationships allow the user of the ONLP APIs to enumerate the system topolology as a directed acyclic graph at any given time.
OID relationships can change over time -- perhaps you remove an existing PSU which contains a temperature sensor and replace it with a PSU that does not.


## OID Interactions

It must be possible to write an application which can operate on any ONLP enabled-hardware, regardless of the number and type of objects present
on any given platform.

Applications discover the OID topology by using the API to enumerate some or all of the current OID tree and interact with the discovered objects.

The specific type of each OID (Fan, PSU, etc) is known at the time of discovery (see Representation above) and as such it can be submitted
to the type-specific API for further programming.

## Common OID Features

In general all OID types have the following features exposed by their APIs.

### The OID Header

All OIDs support a common OID header structure (onlp_oid_hdr_t) which describes generic attributes for that object, such as:
* The OID to which this structure belongs
* The description of this object, such as
  * "QSFP Thermal Sensor"
  * "Fan Tray 4, Front Fan"
* The parent of this OID

Every class of OID has an accessor function to query that OID's header data in the form of:
```
int onlp_<type>_hdr_get(onlp_oid_t, onlp_oid_hdr_t*);
```

### The Information Structure

All OIDs have an information structure that can be queried which provides the current status and capabilities of the object.
This structure is defined in the form of:
```
typedef struct {
        onlp_oid_hdr_t hdr;
        /* Fields specific this particular type */
} onlp_<type>_info_t;
```

This structure is queried through an API in the form us:
```
int onlp_<type>_info_get(onlp_oid_t, onlp_<type>_info_t*);
```

### JSON Representations

All OID Information structures can be marshalled to/from JSON, either individually or recursively. All fields are preserved between C and JSON in the transaction.

Information structures are converted to JSON through an API in the form of:
```
int onlp_<type>_info_to_json (onlp_<type>_info_t*, cJSON**, uint32_t flags);
```

Information structures are populated from JSON through an API in the form of:
```
int onlp_<type>_info_from_json (cJSON*, onlp_<type>_info_t*);
```

### User JSON Representations

All OID Information structures can produce a "User" JSON representation which produces a version of the information suitable for communication to the system user.
This representation takes into account the capabilities of the object as well as the valididity of the fields in the information structure.

The regular JSON structure provides complete fidelity with the the C structure but obviously contains information specific to the APIs.

The User JSON interprets the flags and capabilities and produces a human-readable format that defines what the status of the object is.
This representation is used to communicate the status to the user.

While the regular JSON interchange is lossless this version is not.

Getting this representation is done thorough an API in the form of:
```
int onlp_<type>_info_to_user_json (onlp_<type>_info_t*, cJSON **, uint32_t flags);
```

There is no ```from_user_json``` as the representation is not sufficient to reconstruct the information structure.

---
[Next: APIs](http://opencomputeproject.github.io/OpenNetworkLinux/onlp/applications/apis)