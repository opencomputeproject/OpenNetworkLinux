# ONLP Overview

The Open Network Linux Platform APIs provide a common, consistent abstraction interface for accessing important platform assets such as SFPs, PSUs, Fans, Thermals, LEDs, and ONIE TLV storage devices.

All Open Networking platforms contain some subset of these objects but the programming requirements to access them are highly dependent upon the hardware design and knowledge thereof.
The goal of the ONLP APIs is to both standardize access to these objects at a functional level for system applications and provide a common implementation framework for hardware developers.

## ONLP APIs For Applications

The ONLP API documentation for Application, System, and Dataplane developers is available [here]({{ site.baseurl }}{% link onlp/applications.md %}

The ONLP API documentation for Platform developers who which to implement support for their platform is available [here]({{ site.baseurl }}{% link onlp/implementors.md %}
