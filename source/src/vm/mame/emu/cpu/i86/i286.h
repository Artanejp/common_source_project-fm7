/* ASG 971222 -- rewrote this interface */
#pragma once

#ifndef __I286INTF_H__
#define __I286INTF_H__

#include "i86.h"

#define TRAP(fault, code)  (UINT32)(((fault&0xffff)<<16)|(code&0xffff))

#endif /* __I286INTF_H__ */
