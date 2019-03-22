/*
 * ARM paging
 *
 * (C) 2019.03.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>

/*
 * Translation tabls
 *
 * VMSAv7 defines two alternative translation table formats:
 * 
 * - Short-descriptor format
 * - Long-descriptor format
 *
 * Short-descriptor format
 *
 *   This is the original format defined in issue A of this Architeture 
 *   Reference Manual, and is the only format support on implementations 
 *   that do not include the Large Physical Address Extension. It uses 
 *   32-bit descriptor entries in the translation tables, and provides:
 *
 *   • Up to two levels of address lookup.
 *   • 32-bit input address. 
 *   • Output address of up to 40 bits.
 *   • Support for PAs of more than 32 bits by use.
 *   • Support for No access, Client, and Manager domains.
 *   • 32-bit table entry.
 *
 *   The short-descriptor translation table format support a memory map
 *   base on memory sections or pages:
 *
 *   Supersections  Consist of 16MB blocks of memory. Support for 
 *                  Supersection is optional.
 *   Sections       Consist of 1MB blocks of memory.
 *   Large pages    Consist of 64KB blocks of memory.
 *   Small pages    Consist of 4KB blocks of memory.
 *
 *   Supersections, Sections and Large pages map regions of memory using
 *   only a single TLB entry.
 *
 *   When using the Short-descriptor translation table format, two levels
 *   of translation tables are held in memory:
 *
 *   First-level table
 *                  Holds first-level descriptor that contain the base 
 *                  address and
 *
 *                  • translation properties for a Section and Supersections
 *                  • translation properties and pointers to a seconds-level
 *                    table for a Large page or a Small page.
 *
 *   Second-level table
 *                  Hold second-level descriptors that contain the base 
 *                  address and translation properties for a Small page or
 *                  a Large page. With the Short-descriptor format, second-
 *                  level tables can be referred to as Page tables.
 *                  A second-level table requires 1KByte of memory.
 *
 *   In the translation tables, in general, a descriptor is one of:
 *
 *   • an invalid or fault entry
 *   • a page table entry, that points to a next-level translation table
 *   • a page or section entry, that defines the memory properties for the
 *     acccess.
 *   • a reserved format.
 *   
 *   Bits[1:0] of the descriptor give the primary indication of the 
 *   descriptor type.
 *
 *
 * Short-descriptor translation table format descriptors
 *
 *   Each entry in the first-level table describes the mapping of the 
 *   associated 1MB MVB range.
 *
 *   Invalid
 *   31                                                             2 1 0
 *   +--------------------------------------------------------------+-+-+
 *   |                                                              | | |
 *   |                             IGNORED                          |0|0|
 *   |                                                              | | |
 *   +--------------------------------------------------------------+-+-+
 *
 *   Page Table
 *   31                                             9        5 4 3 2 1 0
 *   +--------------------------------------------+-+--------+-+-+-+-+-+
 *   |                                            | |        | | | | | |
 *   |    Page table base address, bits[31:10]    | | Domain | | | |0|1|
 *   |                                            | |        | | | | | |
 *   +--------------------------------------------+-+--------+-+-+-+-+-+
 *                                                 A          A A A
 *                                                 |          | | |
 *                       IMPLEMENTATION DEFINED ---o          | | |
 *                                                     SBZ ---o | |
 *                                                        NS ---o |
 *                                                         PXN ---o
 *
 *
 *  
 *   Section
 *   31                          20  18    15  c  a 9        5 4 3 2 1 0
 *   +---------------------------+-+-+-+-+-+---+--+-+--------+-+-+-+-+-+
 *   |                           | | | | | |   |  | |        | | | | | |
 *   | Section base address      | |0| |S| |   |  | | Domain | |C|B|1| |
 *   | PA[31:20]                 | | | | | |   |  | |        | | | | | |
 *   +---------------------------+-+-+-+-+-+---+--+-+--------+-+-+-+-+-+
 *                                A   A   A  A   A A          A       A
 *                                |   |   |  |   | |          |       |  
 *                          NS ---o   |   |  |   | |    XN ---o       |
 *                              nG ---o   |  |   | |           PXN ---o
 *                               AP[2] ---o  |   | |
 *                               TEX[2:0] ---o   | |
 *                                    AP[1:0] ---o |
 *                       IMPLEMENTATION DEFINED ---o
 *
 *
 *
 *   Inclusion of the PXN attribute in the Short-descriptor translation
 *   table format is:
 *
 *   • OPTIONAL in an implementation that does not include the Large
 *     Physical Address Extension. 
 *   • required in an implementation includes the Large Physical Address
 *     Extension.
 *
 *   Descriptor bits[1:0] identify the descriptor type. On an implementation
 *   that supports the PXN attribute, for the Section and Supersection 
 *   entries, bit[0] also defines the PNX value. The encoding of these
 *   bits is:
 * 
 *   0b00, Invalid
 *
 *         The associated VA is unmapped, and any attempt to access it 
 *         generates a Translation fault. Software can use bits[31:2] of
 *         the descriptor for its own purposes, because the hardware ignores
 *         bits.
 *
 *   0b01, Page table
 *
 *         The descriptor gives the address of a second-level translation 
 *         table, that specifies the mapping of the associated 1MByte VA 
 *         range.
 *
 *   0b10, Section or Supersection
 *
 *         The descriptor gives the base address of the Section or 
 *         Supersection. Bit[18] determines whether the entry describes
 *         a Section or a Supersection.
 *         If the implementation support the PXN attribute, this encoding
 *         also defines the PXN bit as 0.
 *
 *   0b11, Section or Supersection, if the implementation supports the PXN
 *
 *         If an implementation supports the PXN attribute, this encoding is
 *         identical to 0b10, except that it defines the PXN bit as 1.
 *
 *   The address information in the first-level descriptors is:
 *
 *   Page table      Bits[31:10] of the descriptor are bits[31:10] of the
 *                   a Page table.
 *   Section         Bits[31:20] of the descriptor are bits[31:20] of the
 *                   Section.
 *   Supersection    Bits[31:24] of the descriptor are bits[31:24] of the
 *                   address of the Supersection.
 * 
 * Memory attributes in the Short-descriptor translation table format 
 * descriptor
 *
 *   This section describes the descriptor fields other than the descriptor
 *   type field and the address field:
 *
 *   • TEX[2:0],C,B
 * 
 *     Memory region attribute bits. More infomation see another. These
 *     bits are not present in a Page table entry. 
 *
 *   • XN bit
 * 
 *     The Execute-never bit. Determines whether the processor can execute
 *     software from the address region. This bit is not present in a Page
 *     table entry.
 *
 *   • PXN bit, when support
 *   
 *     The Privileged execute-never bit:
 *
 *     > On an implementation that does not include the Large Physical
 *       Address Extension, support for the PXN bit in the Short-descriptor
 *       translation table format if OPTIONAL.
 *     > On an implementation that include the Large Physical Address 
 *       Extension, the Short-descriptor translation table format include
 *       the PXN bit.
 *
 *     When supported, the PXN bit determines whether the processor can 
 *     execute software from the region when executing at PL1.
 *     When this bit is set to 1 in the Page table descriptor, it indicates 
 *     that all memory pages described in the corresponding page table are
 *     Privileged execute-never.
 *
 *   • NS bit
 *
 *     Non-secure bit. If an implementation includes the Security Extensions,
 *     for memory accesses from Secure state, this bit specifies whether the
 *     translated PA is in Secure or Non-secure address map. This bit is not
 *     present in second-level descriptors. The valude of the NS bit in the 
 *     first level Page table descriptor applies to all entries in the 
 *     corresponding second-level translation table.
 *
 *   • Domain
 *   
 *     Domain field. A domain is a collection of memory regions. The Short-
 *     descriptor translation table format supports 16 domains, and requires
 *     the software that defines a translation table to assign each VMSA
 *     memory region to a domain. When using the Short-descriptor format:
 *
 *     > First-level translation table entries for Page tables and Sections
 *       include a domain filed.
 *     > Translation table entries for Supersections do not include a domain
 *       field. The Short-descriptor format defines Supersection as being in
 *       domain 0.
 *     > Second-level translation table entries inherit a domain setting
 *       from the parent first-level Page table entry.
 *     > Each TLB entry includes a domain field.
 *
 *     The domain field specifies which of the 16 domains the entry is in,
 *     and a two-bit field in the DACR defines the permitted access for each
 *     domain. The possible settings for each domain are:
 *
 *     > No access    Any access using the translation table descriptor
 *                    generates a Domain fault.
 *     > Clients      On an access using the translation table descriptor,
 *                    the access permission attribute are checked. Therefore,
 *                    the access might generate a Permission fault.
 *     > Managers     On an access using the translation table descriptor,
 *                    the access permission attributes are not checked.
 *                    Therefore, the access cannot generate a Permission fault.
 *
 *     more information see another. This field is not present
 *     in a Supersection entry. Memory described by Supersections is in 
 *     domain 0.
 *
 *   • An IMPLEMENTATION DEFINED bit
 *
 *     This bit is not present in second-level descriptors.
 *
 *   • AP[2], AP[1:0]
 *
 *     Access Permissions bits. In addition to an output address, a 
 *     translation table entry that refers to page or region of memory
 *     include field that define properties of the target memory region.
 *     The access control fields, described in this section, determine 
 *     whether the processor, in its current state, is permitted to 
 *     perform the required access to the output address given in the 
 *     translation stage, and no memory access is performed.
 *     AP[0] can be configure as the Access flag.
 *
 *   • nG bit
 *
 *     The not global bit. Determines how the translation is marked in the
 *     TLB.
 *   • 
 *   • 
 *   • 
 *   • 
 *   • 
 *   • 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 */

static int debug_paging(void)
{

	return 0;
}
device_initcall(debug_paging);
