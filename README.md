## Overview

To move UPF onto satellites, we plan to refer to the 3GPP specifications for I-UPF, implementing an I-UPF designed for operation above satellites, also known as S-UPF (Satellite UPF). 

The preliminary functionality of this UPF involves removing anchor point capabilities, retaining only routing decision functions, allowing for advanced traffic pre-routing.

To achieve this objective, we have extended the codebase of Open5GS, including the implementation of I-UPF code, as well as modifications to the core network and access network processes.

Open5GS is a C-language Open Source implementation of 5G Core and EPC. (https://github.com/open5gs/open5gs)


## Core network modification

<p align="center">
  <a href="https://github.com/OreoFroyo/open5gs_iupf"><img src="/.github/I-UPF.png" width="900" title="I-UPF"></a>
</p>

Specifically, we made the following modifications to Open5GS:

- Introduced a new I-UPF network function, with its initialization processes aligned with other network functions in Open5GS.
- Implemented the N9 interface.
- Modified the session establishment process in the SMF, adding storage for relevant I-UPF fields and introducing a selection mechanism during session establishment.

## Usage

In open5gs, starting projects through quickstart and building from source is supported. **Currently, our project only supports building from source compilation.**

Use at least three VMS: one running UE and gnb (UERANSIM is recommended), one running I-UPF, and one running core network (UPF and I-UPF cannot run on the same VM, which may cause IP conflict).

Configuration file modification:

The format of the I-UPF config file is the same as that of UPF, so it follows the UPF modification method.
For other NFs modifications, refer to the open5gs official website. You only need to modify the SMF.config additionally:
```
upf:
    pfcp:
      - addr: 192.168.247.001 (your UPF address)
    ipfcp:
      - addr: 192.168.247.002 (your I-UPF address)
```
