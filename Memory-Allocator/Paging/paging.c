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
 *
 *     In a VMSA implementation, system software can divide a virtual memory
 *     map used by memory accesses at PL1 and PL0 into global and non-global
 *     regions, indicated by the nG bit in the translation table descriptors:
 *
 *     nG == 0        The translation is global, meaning the region is 
 *                    available for all processes.
 *     nG == 1        The translation is non-global, or process-specific,
 *                    meaning it relates to the current ASID, as defined by
 *                    the CONTEXTIDR.
 *   • Bit[18]
 *
 *     When bits[1:0] indicate a Section or Supersection descriptor
 *
 *     0              Descriptor is for a Section
 *     1              Descriptor is for a Supersection
 *
 */

static int debug_paging(void)
{

/*
 * Translation table walks, when using the Short-descriptor translation table
 * format.
 *
 *   When using the Short-descriptor translation table format, and a memory
 *   access requires a translation table walk:
 *
 *   •       a section-mapped access only requires a read of the first-level
 *           translation table. 
 *   •       a page-mapped access also requires a read of the second-level
 *           translation table.
 *
 *   Reading a first-level translation table describs how either TTBR1 or 
 *   TTBR0 is used, with the accessed VA, to determine the address of the 
 *   first-level descriptor.
 * 
 *   On a TLB miss, the VMSA must perform a translation table walk, and 
 *   therefore must find the base address of the translation table to use for
 *   its lookup. A TTBR holds this address.
 *
 *   •       For a Non-secure PL1&0 stage 1 translation, either TTBR0 or 
 *           TTBR1 holds the required base address. The TTBCR is the control
 *           register for these translation.
 *
 *   The Non-secure copies of TTBR0, TTBR1, and TTBCR, relate to the 
 *   Non-secure PL1&0 stage 1 translation.
 *   For Non-secure PL1&0 translation table walks:
 *
 *   •       TTBR0 can be configure to describe the translation of VAs in
 *           the entire address map, or to describe only translation of
 *           VAs in the lower part of the address map.
 *   •       If TTBR0 is configured to describe the translation of VAs in
 *           the lower part of the address map, TTBR1 is configured to 
 *           describe the translation of VAs in the upper part of the
 *           address map.
 *
 *   The contents of appropriate copy of the TTBCR determine whether the 
 *   address map is separated into two parts, and where the separation
 *   occurs.
 *
 *   Two sets of translation table can be defined for each of the PL1&0 stage
 *   1 translations, and TTBR0 and TTBR1 hold the base addresses for the two
 *   sets of tables. When using the Short-descriptor translation table format,
 *   the valude of TTBCR.N indicates the number of most significant bits of
 *   the input VA that determine whether TTBR0 or TTBR1 holds the required
 *   translation table base address, as follow:
 *
 *   •       If N == 0 then use TTBR0. Setting TTBCR.N to zero disables use
 *           of a second set of translation tables.
 *   •       If N > 0 then:
 *           
 *           --     If bits[31:32-N] of the input VA are all zero then use
 *                  TTBR0
 *           --     otherwise use TTBR1
 *
 *   Table shows how the value of N determines the lowest address translated
 *   using TTBR1, and the size of the first-level translation table addressed
 *   by TTBR0.
 *
 *   TTBCR.N | First address translated | TTBR0 table | Index range 
 *           | with TTBR1               | size        | 
 *   --------+--------------------------+-------------+------------------
 *   0b000   | TTBR1 not used           | 16KB        | VA[31:20]
 *   0b001   | 0x80000000               | 8KB         | VA[30:20]
 *   0b010   | 0x40000000               | 4KB         | VA[29:20]
 *   0b011   | 0x20000000               | 2KB         | VA[28:20]
 *   0b100   | 0x10000000               | 1KB         | VA[27:20]
 *   0b101   | 0x08000000               | 512 bytes   | VA[26:20]
 *   0b110   | 0x04000000               | 256 bytes   | VA[25:20]
 *   0b111   | 0x02000000               | 128 bytes   | VA[24:20]
 *   --------+--------------------------+-------------+------------------
 *
 *   Whenever TTBCR.N is nonzero, the size of translation table addressed
 *   by TTBR1 is 16KB.
 *
 *   
 *   0xFFFFFFFF +---------------+ - +---------------+
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |
 *              | TTBR0 region  |   | TTBR1 region  |
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   |               |  A
 *              |               |   |               |  |
 *              |               |   |               |  | Effect of decreasing
 *              |               |   |               |  | N Boundary, when
 *   0x02000000-|               | - +---------------+ <----- TTBCR.N == 0b111
 *              |               |   |               |
 *              |               |   |               |
 *              |               |   | TTBR0 region  |
 *              |               |   |               |
 *              |               |   |               |
 *   0x00000000 +---------------+   +---------------+
 *              TTBCR.N == 0b000
 *            Use of TTBR1 disabled
 *
 *   In the selected TTBR. the following bits define the memory region 
 *   attribute table walk:
 *
 *   •       the RGN, S and C bits, in an implementation that does not
 *           include the Multiprocessing Externsions.
 *   •       the RGN, S and IRGN[1:0] bits, in an implementation that
 *           includes the Multiprocessing Externsions. 
 */
	unsigned long TTBCR;
	unsigned long TTBR0;
	unsigned long TTBR1;
	unsigned long SCTLR;
	unsigned long pgd_base;
	unsigned long va = PAGE_OFFSET;
	pgd_t *pgd;

	/* Read TTBCR Register */
	__asm__ volatile ("mrc p15,0,%0,c2,c0,2" : "=r" (TTBCR));
	/* Check TTBCR.N to indicate TTBR0 or TTBR1 */
	if (TTBCR & 0x3) {
		printk("PL1&0 Stage 1 translation using TTBR0 and TTBR1.\n");
		__asm__ volatile ("mrc p15,0,%0,c2,c0,0" : "=r" (TTBR0));
		__asm__ volatile ("mrc p15,0,%0,c2,c0,1" : "=r" (TTBR1));
	} else {
		printk("PL1&0 Stage 1 translation only using TTBR0.\n");
		__asm__ volatile ("mrc p15,0,%0,c2,c0,0" : "=r" (TTBR0));
	}

/*
 * TTBR0 format
 *
 *
 *   31                                  x               6  5    3  2  1  0
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *   |                                   |   Reserved    |  |    |  |  |  |
 *   | Translation table base 0 address  |   UNK/SBZP    |  |RGN |  |S |C |
 *   |                                   |               |  |    |  |  |  |
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *                                                        A       A
 *                                                        |       |
 *                                                NOS-----o       |
 *                                                IMP-------------o
 *
 *
 * TTBR1 format
 *
 *
 *
 *   31                                  x               6  5    3  2  1  0
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *   |                                   |   Reserved    |  |    |  |  |  |
 *   | Translation table base 1 address  |   UNK/SBZP    |  |RGN |  |S |C |
 *   |                                   |               |  |    |  |  |  |
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *                                                        A       A
 *                                                        |       |
 *                                                NOS-----o       |
 *                                                IMP-------------o
 *
 *
 * Reading a first-level translation table
 *
 *   When performing a feth base on TTBR0:
 *
 *   •       the address bits taken from TTBR0 vary between bits[31:14] and
 *           bits[31:7].
 *   •       the address bits taken from the VA, that is the input address
 *           for the translation, vary between bits[31:20] and bits[24:20].
 *
 *   The width of the TTBR0 and VA fields depend on the value od TTBCR.N.
 *   When performing a fetch based on TTBR1, Bits TTBR1[31:4] are 
 *   concatenated with bits[31:20] of the VA. This makes the fetch equivalent
 *   to that show in Figure.
 *
 *
 *                   Input address
 *
 *                   32                20                                 0
 *                   +-----------------+----------------------------------+
 *                   |   Table index   |                                  |
 *                   +-----------------+----------------------------------+
 *                   | <-------------> |
 *                           |
 *                           |
 *                           o-------------------------------o
 *                                                           |
 * TTBR0:                                                    |
 *                                                           |
 * 32                           14         7            0    |
 * +----------------------------+----------+------------+    |
 * |      Translation base      | UNK/SBZP | Properties |    |
 * +----------------------------+----------+------------+    |
 * | <------------------------> |                            |
 *               |                                           |
 *               |                                           |
 *               o------------------o                        |
 *                                  |                        |
 * Descriptor address:              |                        |
 *                   32             V               14       V        2    
 *                   | <--------------------------> | <-------------> |   0
 *                   +------------------------------+-----------------+-+-+
 *                   |       Translation base       |   Table index   |0|0|
 *                   +------------------------------+-----------------+-+-+
 *    
 */
	pgd_base = (((TTBR0 >> 14) & 0x3FFFF) << 14) | 
			(((va >> 20) & 0xFFF)) << 2;
	printk("%#lx's PageDirent Entry: %#lx\n", va, pgd_base);

/*
 *   Regardless of which register is used as the base for the fetch, the 
 *   resulting output address selects a four-byte translation table entry
 *   that is one of:
 *
 *   •       A first-level descriptor for a Section or Supersection.
 *   •       A Page table describe that points to a second-level translation
 *           table. In this case:
 *
 *           >       a second fetch is performed to retrieve a second-level
 *                   descriptor.
 *           >       the descriptor also contains some attribute for the 
 *                   descriptor.
 *   •       A faulting entry.
 *
 * The full translation flow for Section, Supersection, Small pages and 
 * Large pages
 *
 *   In a translation table walk, only the first lookup uses the translation
 *   table base address from the appropriate Translation table base register.
 *   Subsequent lookups use a combination of address information from:
 *
 *   •       the table descriptor read in the previous lookup
 *   •       the input address.
 *
 *   Here summarizes how each of the memory section and page options is 
 *   described in the translation tables, and has a subsection summarizing
 *   the full translation flow for each of the options.
 *   
 *   Supersection    A 16MB memory region.
 *   Section         A 1MB memory region.
 *   Large page      A 64KB memory region, described by the combination of:
 *
 *                   >      a first-level translation table entry that
 *                          indicates a second-level Page table address.
 *                   >      a second-level descriptor that indicates a 
 *                          Large page.
 *   Small page      A 4KB memory region, described by the combination of:
 *
 *                   >      a first-level translation table entry that 
 *                          indicates a second-level Page table address.
 *                   >      a second-level descriptor that indicates a
 *                          Small page.
 *
 *   The address is the Physical address of the desciptor, Section,
 *   Supersection, Large page, or Small page.
 */
	pgd = (pgd_t *)__va(pgd_base);
	printk("Content of first-level descriptor: %#lx\n", 
					(unsigned long)pgd_val(*pgd));

	if (!(pgd_val(*pgd) & 0x3)) {
		printk("First-level descriptor is invalid!\n");
	} else if ((pgd_val(*pgd) & 0x3) == 0x1) {
		printk("First-level descriptor is Page table!\n");
	} else if ((pgd_val(*pgd) & 0x2)) {
		/* Section or Supersection 
		 *   The descriptor gives the base address of the Section or
		 *   Supersection. Bit[18] determines whether the entry 
		 *   describes a Section or a Supersection.
		 */
		if ((pgd_val(*pgd) >> 18) & 0x1) {
			printk("First-level descriptor is Supersection!\n");
		} else {
			printk("First-level descriptor is Section!\n");
		}
	}

/*
 *   Properties indicates register or translation table fields that return
 *   information, other than address information, about the translation or the
 *   targeted memory region. In a VMSA implementation, when an associated MMU
 *   is enable, a memory access requires one or more translation table lookups.
 *   If the required translation table descriptor is not held in a TLB, a 
 *   translation table walk is performed to obtian the descriptor. A lookup,
 *   whether from the TLB or as the result of a translation table walk, returns
 *   both:
 *
 *   •       an output address that corresponds to the input address for the
 *           lookup
 *   •       a set of properties that correspond to that output address.
 *
 *   The returned properties are classified as providing address map control,
 *   access controls, or region attributes. This classification determines
 *   how the descriptions of the properties are grouped. The classification 
 *   is based on the following model:
 *
 *   Address map control
 *
 *           Memory accesses from Secure state can access either the Secure
 *           or the Non-secure address map, as summarized in Access to the
 *           Secure or Non-secure physical address map. Memory accesses from
 *           Non-secure state can only access the Non-secure address map.
 *
 *   Access controls
 *
 *           Determine whether the processor, in its current state, can 
 *           access the output address that corresponds to the given input
 *           address. If not, an MMU fault is generated and there is no 
 *           memory access. Memory access control describes the properties
 *           in this group.
 *
 *   Attributes
 *
 *           Are valid only for an output address that the processor, in its
 *           current state, can access. The attributes define aspects of the
 *           required behavior of accesses to the target memory region.
 *
 * Access to the Secure or Non-secure physical address map
 *
 *   As stated in Address space in a VMAS implementation, a processor that
 *   implementats that Security Extensions implements independent Secure and
 *   Non-secure address maps. These are defined by the translation tables
 *   identified by the Secure TTBR0 and TTBR1. In both translation table
 *   formats:
 *
 *   •       In the Non-secure translation tables, the corresponding bit is
 *           SBZ. Non-secure physical address space, regardless of the value
 *           of this bit.
 *
 * Memory access control
 *
 *   In addition to an output address, a tranlation table entry that refers 
 *   to page or region of memory includes field that define properites of
 *   the target memory region. The access control field, described in here,
 *   determint whether the processor, in its current state, is permitted to
 *   perform the required access to the output address given in the 
 *   translation table descriptor. If a translation stage does not permit the
 *   access then an MMU fault is generated for that translation stage, and no
 *   memory access is performed.
 *
 * Access permissions
 *
 *   Access permission bits in a translation table descriptor control access
 *   to the corresponding memory region. The Short-descriptor translation
 *   table format support two options for defining the access permissions:
 *
 *   •       three bits, AP[2:0], define the access permissions.
 *   •       two bits, AP[2:1], define the access permissions, and AP[0]
 *           can be used as an Access flag.
 *
 *   SCTLR.AFE selects the access permission option. Setting this bit to 1, 
 *   to enable the Access flag, also selects use of AP[2:1] to define access
 *   permissions.
 */
	__asm__ volatile ("mrc p15,0,%0,c1,c0,0" : "=r" (SCTLR));
	/* Detect SCTLR.AFE bit 
	 *   Access flag enable. The possible value of this bit are:
	 *
	 *   0      In the translation table descriptors, AP[0] is an access
	 *          permission bit. The full range of access permission is 
	 *          supported. No Access flag is implemented.
	 *   1      In the translation table descriptors, AP[0] is the 
	 *          Access flag. Only the simolified model for access 
	 *          permissions is supported.
	 */
	if ((SCTLR >> 29) & 0x1) {
		/* Setting this bit to 1 enable use of the AP[0] bit in the
		 * translation table descriptors as the Access flag. It also
		 * restricts access permission in the translation table 
		 * descriptors to the simplified model.
		 */
		printk("AP[0] is access bit.\n");
	} else {
		printk("AP[0] is access permission bit.\n");
	}

/*
 * AP[2:1] access permission model
 *
 *   ---Note---
 *   Some documentation describes this as the simplified access permission
 *   model.
 *   ----------
 *
 *   This access permissions model is used if the translation is either:
 *
 *   •       using the Long-descriptor translation table format
 *   •       using Short-descriptor translation table format, and the
 *           SCTLR.AFE bit is set to 1.
 *
 *   In this model:
 *
 *   •       One bit, AP[2], selects between read-only and read/write access.
 *   •       A second bit, AP[1], selects between Application level (PL0)
 *           and System level (PL1) control. For the Non-secure PL2 stage 1
 *           translations, AP[1] is SBO.
 *
 *   In the ARM architecutre, this model permits four access combinations:
 *
 *   •       read-only at all privilege levels
 *   •       read/write at all privilege levels
 *   •       read-only at PL1, no access by software executing at PL0
 *   •       read/write at PL1, no access by software executing at PL0.
 *
 *   ---------------+---------------------+-----------------------------------
 *   AP[2], disable | AP[1], enable       | Access
 *   write access   | unprivileged access |
 *   ---------------+---------------------+-----------------------------------
 *   0              | 0(a)                | Read/Write, only at PL1
 *   0              | 1                   | Read/Write, at any privilege level
 *   1              | 0(a)                | Read-only, only at PL1
 *   1              | 1                   | Read-only, at any privilege level
 *   ---------------+---------------------+-----------------------------------
 *   a. Not valid for Non-secure PL2 stage 1 translation tables. AP[1] is SBO
 *      in these tables.
 *
 * AP[2:0] access permission
 *
 *   This access permission model applies when using the Short-descriptor 
 *   translation tables format, and the SCTLR.AFE bit is set to 0. Table show
 *   this access permiision model.
 *
 *   When SCTLR.AFE is set to 0, ensuring that the AP[0] bit is always set to
 *   1 effectively changes the access model to the simpler model.
 *
 *   ------+---------+-------------+--------------+--------------------------
 *   AP[2] | AP[1:0] |  PL access  | Unprivileged | Description
 *         |         |             | access       |
 *   ------+---------+-------------+--------------+--------------------------
 *   0     | 00      | No access   | No access    | All access generate
 *         |         |             |              | Permission faults
 *   ------+---------+-------------+--------------+--------------------------
 *   0     | 01      | Read/write  | No access    | Access only at PL1
 *   ------+---------+-------------+--------------+--------------------------
 *   0     | 10      | Read/write  | Read-only    | Write at PL0 generate
 *         |         |             |              | Permission faults 
 *   ------+---------+-------------+--------------+--------------------------
 *   0     | 11      | Read/write  | Read/write   | Full access
 *   ------+---------+-------------+--------------+--------------------------
 *   1     | 00      | -           | -            | Reserved
 *   ------+---------+-------------+--------------+--------------------------
 *   1     | 01      | Read-only   | No access    | Read-only, only at PL1
 *   ------+---------+-------------+--------------+--------------------------
 *   1     | 10      | Read-only   | Read-only    | Read-only at any
 *         |         |             |              | privilege level
 *   ------+---------+-------------+--------------+--------------------------
 *   1     | 11      | Read-only   | Read-only    | Read-only at any
 *         |         |             |              | privilege level.
 *   ------+---------+-------------+--------------+--------------------------
 *
 * AP[] at Section descriptor
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
 */
	if ((pgd_val(*pgd) >> 15) & 1) {
		switch ((pgd_val(*pgd) >> 10) & 0x3) {
		case 0x00:
			printk("-\n");
			printk("Reserved.\n");
			break;
		case 0x01:
			printk("Read-only.\n");
			printk("Read-ony, only at PL1.\n");
			break;
		case 0x02:
			printk("Read-only.\n");
			printk("Read-only at any privilege level.\n");
			break;
		case 0x03:
			printk("Read-only.\n");
			printk("Read-only at any privilege level.\n");
			break;
		}
	} else {
		switch ((pgd_val(*pgd) >> 10) & 0x3) {
		case 0x00:
			printk("Section No access.\n");
			printk("All accesss generate Permission faults.\n");
			break;
		case 0x01:
			printk("Section Read/write.\n");
			printk("Access only at PL1\n");
			break;
		case 0x02:
			printk("Section Read/write.\n");
			printk("Write at PL0 generate Permission faults.\n");
			break;
		case 0x3:
			printk("Section Read/write.\n");
			printk("Faull access.\n");
			break;
		}	
	}

/*
 * The Access flag
 *
 *   The Access flag indicates when a page or section of memory is accessed
 *   for the first time since the Access flag in the corresponding 
 *   translation table descriptor was set to 0:
 *
 *   •       If address translation is using the Short-descriptor translation
 *           table format, it must set SCTLR.AFE to 1 to enable use of the 
 *           Access flag. Setting this bit to 1 redefines the AP[0] bit in 
 *           the translation table descriptors as an Access flag, and limits
 *           the access permissions information in the translation table
 *           descriptors to AP[2:1].
 *
 *   •       The Long-descriptor format always supports an Access flag bit 
 *           the translation table descriptors, and address translation using
 *           this format behaves at if SCTLR.AFE is set to 1, regardless of
 *           the value of that bit.
 *
 *   The Access flag can be managed by software or by hardware. However,
 *   support for hardware management of the Access flag is OPTIONAL and 
 *   deprecated. The following subsections describe be management options:
 *
 *   •       Software management of the Access flag
 *   •       Hardware management of the Access flag
 *
 * Software management of the Access flag
 *
 *   An implementation that requires software to manage the Access flag
 *   generates an Access flag fault whenever a translation table entry with
 *   the Access flag set to 0 is read into TLB.
 *
 *   The Access flag mechanism expects that, when an Access flag fault occurs,
 *   software resets the Access flag to 1 in the translation table entry that
 *   caused the fault. This prevents the fault occurring the next time that
 *   memory location is accessed. Entries with the Access flag set to 0 are
 *   never held in the TLB, meaning software does not have to flush the entry
 *   from the TLB after setting the flag.
 *
 * Hardware management of the Access flag
 *
 *   For the Secure and Non-secure PL1&0 stage 1 translations, an 
 *   implementation can provide hardware management of the Access flag.
 *   In this case, if a translation table entry with the Access flag set to
 *   0 is read into the TLB, the hardware writes 1 to the Access flag bit of
 *   the translation table etnry in memory.
 *
 *   An implementation that provides hardware management of the Access flag
 *   for the Secure and Non-secure PL1&0 stage 1 translations:
 *
 *   •       Uses the HW Access flag field, ID_MMFR2[31:28], to indicate this
 *           implementation choice.
 *   •       Implements the SCTLR.HA bit. This bit must be set to 1 enable 
 *           hardware management of the Access flag.
 * 
 *   ---Note---
 *   When using the Short-descriptor translation table format, hardware 
 *   management of the Access flag is performed only if both:
 *   
 *   •       SCTLR.AFE is set to 1, to enable use of an Access flag.
 *   •       SCTLR.HA i set to 1, to enable hardware management of the Access
 *           flag. 
 *   ----------
 * 
 *   When hardware management of the Access flag, is enabled for a stage of
 *   address translation, no Access flag faults are generated for the 
 *   corresponding translations.
 *
 *   Any implementation of hardware management of the Access flag must ensure
 *   that any software changes to the translation table are not overwritten.
 *   The architecture does not require software that changes translation table
 *   entries to use interlocked operations. The hardware management mechanisms
 *   for the Access flag must prevent any loss of data written to translation
 *   table entries that might occur when, for example, a write by another
 *   processor occurs between the read and write phases of a translation
 *   table walk that updates the Access flag.
 *   Architecturally, an operating system that uses the Access flag must 
 *   support the software faulting option that generates Access flag faults.
 *   This provides compatibility between systems that include a hardware
 *   implementation of the Access flag and those systems that do not implement
 *   this feature.
 * 
 *   ARM deprecates any use of the SCTLR.HA bit. That is, in an implementation
 *   where this bit is RW, it deprecates setting this bit to 1 to enable 
 *   hardware management of the Access flag.
 *
 *   Hardware management of the Access flag is never supported for:
 *
 *   •       Non-secure PL1&0 stage 2 translations
 *   •       Non-secure PL2 stage 1 translations.
 */
	if (!((SCTLR >> 29) & 0x1)) {
		printk("First-level descriptor doesn't support Access bit.\n");
	} else {
		printk("First-level descriptor support Access bit.\n");
		if ((SCTLR >> 17) & 0x1) {
			printk("Support Hardware Access bit.\n");
		} else {
			printk("Support software Access bit.\n");
		}
	}

/*
 * Execute-never restrictions on instruction fetching
 *
 *   Execute-never (XN) controls provide an additional level of control on
 *   memory accesses permitted by the access permissions settings. These
 *   controls are:
 *
 *   XN,Execute-never
 *             When the XN bit is 1, a Permission fault is generated if the
 *             processor attempts to execute an instruction fetched from the
 *             corresponding memory region. However, when using the 
 *             Short-descriptor translation table format, the fault is
 *             generated only if the access is to memory in the Client domain,
 *             A processor can execute instruction from a memory region
 *             only if the access permissions for its current state permit
 *             read access, and the XN bit is set to 0.
 *
 *   PXN,Privileged execute-never
 *             When the PXN bit is 1, a Permission fault is generated if the
 *             processor is executing at PL1 and attempts to execute an 
 *             instruction fetched from the corresponding memory region. As
 *             with the XN bit, when using the Short-descriptor translation
 *             table format, the fault is generated only if the access is to
 *             memory in the Client domain.
 *
 *   In both the Short-descriptor format, all descriptors for memory blocks
 *   and pages always include an XN bit. On an implementation that doen not
 *   include the Large Physical Address Extension, support for use of the PXN
 *   bit is OPTIONAL, and:
 *
 *   --        if use of the PXN bit is supported, the Short-descriptor 
 *             translation table formats include the PXN bit.
 *   --        otherwise, the Short-descriptor translation table formats
 *             do not include the PXN bit.
 *
 *
 *   XN/PXN on Section:
 *
 *
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
 *   The execute-never controls apply also to speculative instruction
 *   fetching. This means a speculative instruction fetch from a memory
 *   region that is execute-never at the current level of privilege is
 *   prohibited. The XN control meanus that, when the MMU is enabled, the
 *   processor can fetch, or speculatively fetch, an instruction from a
 *   memory location only if all of the following apply:
 *
 *   •       If using the Short-descriptor translation table format, the
 *           translation table descriptor for the location does not 
 *           indicate that it is in a No access domain.
 *   •       If using the Long-descriptor translation table format, or
 *           using the Short descriptor for the location and the descriptor
 *           indicates that the location is in a Client domain, in the 
 *           descriptor for the location the following apply:
 *
 *           --          XN is set to 0
 *           --          the access permissions permit a read access from
 *                       the current processor mode.
 *   •       No other Prefetch Abort condition exists.
 */
	if ((pgd_val(*pgd) >> 4) & 0x1) {
		printk("AAAA\n");
	} else
		printk("BBBB\n");

	return 0;
}
device_initcall(debug_paging);
